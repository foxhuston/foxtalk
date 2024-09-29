use crate::query::Query;
use crate::tuple::{Tuple, TupleNoun};
use libc::c_char;
use std::ffi::{c_void, CStr, CString};
use std::mem;
use std::ptr::{null_mut, NonNull};

use anyhow::{format_err, Result};

use libloading::os::unix::{Library, Symbol};
use crate::bindings;
use crate::bindings::{std_nullptr_t, TupleNoun_Tag};
use crate::when::When;

impl From<crate::bindings::TupleNoun> for TupleNoun {
    fn from(value: crate::bindings::TupleNoun) -> Self {
        unsafe {
            match value.tag {
                crate::bindings::TupleNoun_Tag_Ptr => {
                    TupleNoun::CPtr(NonNull::new(value.dat.ptr).unwrap())
                }
                crate::bindings::TupleNoun_Tag_Str => {
                    TupleNoun::Str(unsafe { CStr::from_ptr(value.dat.str_).to_string_lossy().into() })
                }
                crate::bindings::TupleNoun_Tag_U64 => {
                    todo!()
                }
                crate::bindings::TupleNoun_Tag_I64 => {
                    todo!()
                }

                _ => unreachable!()
            }
        }
    }
}

impl From<crate::bindings::Tuple> for Query {
    fn from(value: crate::bindings::Tuple) -> Self {
        unsafe {
            let p = if value.predicate.is_null() { None } else { Some(unsafe { CStr::from_ptr(value.predicate) }) };
            let predicate = p.map(|s| s.to_str().unwrap().to_owned());

            Query {
                subject: if value.subject.is_null() { None } else { Some((*value.subject).into()) },
                predicate: predicate,
                object: if value.object.is_null() { None } else { Some((*value.object).into()) },
            }
        }
    }
}

impl Default for crate::bindings::Tuple {
    fn default() -> Self {
        crate::bindings::Tuple {
            subject: null_mut(),
            predicate: null_mut(),
            object: null_mut(),
        }
    }
}

impl Tuple {
    unsafe fn from_ctuple(free_subj: NonNull<c_void>, free_obj: NonNull<c_void>, value: crate::bindings::Tuple) -> Self {
        assert!(!value.subject.is_null());
        assert!(!value.object.is_null());

        let s: TupleNoun = (*value.subject).into();
        let o: TupleNoun = (*value.object).into();

        let s = match s {
            TupleNoun::CPtr(p) => {
                TupleNoun::CPtrHeap { data: p, destructor: free_subj }
            }
            o => o
        };

        let p: String = unsafe { CStr::from_ptr(value.predicate).to_string_lossy().into() };

        let o = match o {
            TupleNoun::CPtr(p) => {
                TupleNoun::CPtrHeap { data: p, destructor: free_subj }
            }
            o => o
        };

        Tuple {
            subject: s,
            predicate: p,
            object: o
        }
    }
}

impl From<TupleNoun> for crate::bindings::TupleNoun {
    fn from(value: TupleNoun) -> Self {
        match value {
            TupleNoun::CPtr(data) |
            TupleNoun::CPtrHeap { data, destructor: _ } => {
                crate::bindings::TupleNoun {
                    tag: crate::bindings::TupleNoun_Tag_Ptr,
                    dat: crate::bindings::TupleNoun__bindgen_ty_1 {
                        ptr: data.as_ptr()
                    }
                }
            }
            TupleNoun::Str(s) => {
                crate::bindings::TupleNoun {
                    tag: crate::bindings::TupleNoun_Tag_Str,
                    dat: crate::bindings::TupleNoun__bindgen_ty_1 {
                        str_: unsafe { mem::transmute(s.clone().as_mut_ptr()) }
                    }
                }
            }
        }
    }
}

type CWhenHandler = unsafe extern "C" fn(*const crate::bindings::Tuple, *mut usize) -> *mut crate::bindings::Tuple;
type CGetQuery = unsafe extern "C" fn(&mut crate::bindings::Tuple) -> ();
type CFreeTuple = unsafe extern "C" fn(&mut crate::bindings::Tuple) -> ();

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
            lib, get_query, when_handler, free_tuple_subj, free_tuple_obj,
        })
    }
}

impl When for CWhen {
    fn get_query(&self) -> Query {
        // Parens necessary, apparently.
        let mut cq = crate::bindings::Tuple::default();
        unsafe { (&self.get_query)(&mut cq) };

        println!("DEBUG! Got CQuery {cq:?}");

        cq.into()
    }

    fn handle(&mut self, results: Tuple) -> Vec<Tuple> {
        let mut s: crate::bindings::TupleNoun = results.subject.into();
        let mut o: crate::bindings::TupleNoun = results.object.into();
        let mut p = unsafe { mem::transmute(results.predicate.clone().as_mut_ptr()) };

        let query_result_tuple = crate::bindings::Tuple {
            subject: &mut s,
            predicate: p,
            object: &mut o
        };

        let mut out_size: usize = 0;
        let ctuple_array = unsafe { (&self.when_handler)(&query_result_tuple, &mut out_size) };

        if ctuple_array.is_null() { return vec![]; }

        let ctuples = unsafe { Vec::from_raw_parts(ctuple_array, out_size, out_size) };

        ctuples.into_iter().map(|tuple| {
            unsafe { Tuple::from_ctuple(self.free_tuple_subj, self.free_tuple_obj, tuple) }
        }).collect()
    }
}