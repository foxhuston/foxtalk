use std::ffi::{c_void, CStr, CString};
use std::fmt::{write, Debug, Formatter, Pointer};
use std::mem;
use std::ptr::NonNull;

use anyhow::{format_err, Result};

use crate::{
    when::When,
    tuple::{Tuple, TupleNoun},
    query::Query,
    bindings
};
use libloading::os::unix::{Library, Symbol};
use crate::bindings::TupleNoun_Tag;

impl From<bindings::TupleNoun> for Option<TupleNoun> {
    fn from(value: bindings::TupleNoun) -> Self {
        unsafe {
            match value.tag {
                bindings::TupleNoun_Tag_Query => {
                    None
                }
                bindings::TupleNoun_Tag_Ptr => {
                    Some(TupleNoun::CPtr(NonNull::new(value.dat.ptr).unwrap()))
                }
                bindings::TupleNoun_Tag_Str => {
                    Some(TupleNoun::Str(CStr::from_ptr(value.dat.str_).to_string_lossy().into()))
                }
                bindings::TupleNoun_Tag_U64 => {
                    todo!()
                }
                bindings::TupleNoun_Tag_I64 => {
                    todo!()
                }

                _ => unreachable!()
            }
        }
    }
}

impl From<bindings::Tuple> for Query {
    fn from(value: bindings::Tuple) -> Self {
        let p = if value.predicate.is_null() { None } else { Some(unsafe { CStr::from_ptr(value.predicate) }) };
        let predicate = p.map(|s| s.to_str().unwrap().to_owned());

        Query {
            subject: value.subject.into(),
            predicate,
            object: value.object.into(),
        }
    }
}

impl Debug for bindings::TupleNoun {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        use bindings::*;

        unsafe {
            match self.tag {
                TupleNoun_Tag_Query => { write!(f, "CQuery") }
                TupleNoun_Tag_Ptr => { write!(f, "CPtr({:?})", self.dat.ptr) }
                TupleNoun_Tag_Str => {
                    let s = CStr::from_ptr(self.dat.str_).to_string_lossy();
                    write!(f, "CStr({})", s)
                }
                TupleNoun_Tag_U64 => { write!(f, "CU64({:?})", self.dat.u64_) }
                TupleNoun_Tag_I64 => { write!(f, "CI64({:?})", self.dat.i64_) }

                _ => unreachable!()
            }
        }
    }
}

impl Debug for bindings::Tuple {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("CTuple")
            .field("subject", &self.subject)
            .field("predicate", &self.predicate)
            .field("object", &self.object)
            .finish()
    }
}

impl Tuple {
    unsafe fn from_ctuple(free_subj: NonNull<c_void>, free_obj: NonNull<c_void>, value: bindings::Tuple) -> Self {
        assert_ne!(!value.subject.tag, bindings::TupleNoun_Tag_Query);
        assert_ne!(!value.object.tag, bindings::TupleNoun_Tag_Query);

        let s: Option<TupleNoun> = value.subject.into();
        let s = s.unwrap();
        let o: Option<TupleNoun> = value.object.into();
        let o = o.unwrap();

        let s = match s {
            TupleNoun::CPtr(p) => {
                TupleNoun::CPtrHeap { data: p, destructor: free_subj }
            }
            o => o
        };

        let p: String = unsafe { CStr::from_ptr(value.predicate).to_string_lossy().into() };

        let o = match o {
            TupleNoun::CPtr(p) => {
                TupleNoun::CPtrHeap { data: p, destructor: free_obj }
            }
            o => o
        };

        Tuple {
            subject: s,
            predicate: p,
            object: o,
        }
    }
}

impl From<TupleNoun> for bindings::TupleNoun {
    fn from(value: TupleNoun) -> Self {
        match value {
            TupleNoun::CPtr(data) |
            TupleNoun::CPtrHeap { data, destructor: _ } => {
                bindings::TupleNoun {
                    tag: bindings::TupleNoun_Tag_Ptr,
                    dat: bindings::TupleNoun_Dat {
                        ptr: data.as_ptr()
                    },
                }
            }

            TupleNoun::Str(s) => {
                bindings::TupleNoun {
                    tag: bindings::TupleNoun_Tag_Str,
                    dat: bindings::TupleNoun_Dat {
                        str_: CString::new(s).unwrap().into_raw()
                    },
                }
            }
        }
    }
}

type CWhenHandler = unsafe extern "C" fn(*const bindings::Tuple, *mut usize) -> *mut bindings::Tuple;
type CGetQuery = unsafe extern "C" fn(&mut bindings::Tuple) -> ();
type CFreeTuple = unsafe extern "C" fn(&mut bindings::Tuple) -> ();

////////////////////////////////////////////////////////////////////////////////

#[allow(dead_code)]
pub struct CWhen {
    lib: Library,
    get_query: Symbol<CGetQuery>,
    when_handler: Symbol<CWhenHandler>,
    free_tuple_subj: NonNull<c_void>,
    free_tuple_obj: NonNull<c_void>,
}

impl CWhen {
    pub unsafe fn new(so_path: &str) -> Result<Self> {
        let lib = Library::new(so_path)?;

        let get_query: Symbol<CGetQuery> = lib.get(b"get_query")?;
        let when_handler: Symbol<CWhenHandler> = lib.get(b"when_handler")?;
        let free_tuple_subj: Symbol<CFreeTuple> = lib.get(b"free_tuple_subj")?;
        let free_tuple_obj: Symbol<CFreeTuple> = lib.get(b"free_tuple_obj")?;

        let free_tuple_subj = NonNull::new(free_tuple_subj.into_raw())
            .ok_or(format_err!("free_tuple_subj to be nonnull"))?;
        let free_tuple_obj = NonNull::new(free_tuple_obj.into_raw())
            .ok_or(format_err!("free_tuple_obj to be nonnull"))?;

        Ok(CWhen {
            lib,
            get_query,
            when_handler,
            free_tuple_subj,
            free_tuple_obj,
        })
    }
}

impl When for CWhen {
    fn get_query(&self) -> Query {
        // Parens necessary, apparently.
        let mut cq = bindings::Tuple::default();
        println!("DEBUG! Input query {cq:?}");
        unsafe { (&self.get_query)(&mut cq) };
        println!("DEBUG! Output query {cq:?}");

        cq.into()
    }

    fn handle(&mut self, results: Tuple) -> Vec<Tuple> {
        println!("C <--> Rust handler begin for:");
        println!("    RUST tuple {results:?}");

        let s: bindings::TupleNoun = results.subject.into();
        let o: bindings::TupleNoun = results.object.into();
        let p = unsafe { mem::transmute(results.predicate.clone().as_mut_ptr()) };

        let query_result_tuple = bindings::Tuple {
            subject: s,
            predicate: p,
            object: o,
        };

        println!("    Using !!C!! Tuple {query_result_tuple:?}");

        let mut out_size: usize = 0;
        let ctuple_array = unsafe { (&self.when_handler)(&query_result_tuple, &mut out_size) };

        if ctuple_array.is_null() { return vec![]; }

        let ctuples = unsafe { Vec::from_raw_parts(ctuple_array, out_size, out_size) };

        let out = ctuples.into_iter().map(|tuple| {
            unsafe { Tuple::from_ctuple(self.free_tuple_subj, self.free_tuple_obj, tuple) }
        }).collect();

        println!("C <--> Rust handler end");

        out
    }
}