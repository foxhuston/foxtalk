mod ffi_blob;
mod conversions;
mod f_tuple;

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

/*
 * The big TODO List:
 *   - First & Foremost, we're converting strings back and forth quite a lot, and
 *     I haven't really been keeping track of who's supposed to free them. That is,
 *     both Rust and C++ can generate strings that both of them need to read, and right now
 *     I'm just leaking most (if not all) of them.
 *   - I also need a better way to do the "wishes" tuples being returned from the handlers.
 *     These are also not cleaned up by anything.
 */

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
        // println!("DEBUG! Input query {cq:?}");
        unsafe { (&self.get_query)(&mut cq) };
        // println!("DEBUG! Output query {cq:?}");

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