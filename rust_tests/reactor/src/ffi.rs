use std::ffi;
use std::ffi::{c_void, CString};
use std::rc::Rc;
use crate::query::Query;
use crate::tuple::Tuple;
use crate::when::When;

use anyhow::Result;
use crate::tuple::TupleNoun;

#[repr(C)]
struct CTuple {
    pub subject: *const c_void,
    pub predicate: CString,
    pub object: *const c_void,
}

#[repr(C)]
struct CQuery {
    pub subject: Option<*const c_void>,
    pub predicate: Option<CString>,
    pub object: Option<*const c_void>,
}

struct CWhen {
    lib: libloading::Library,
    // c_get_query: libloading::Symbol<'a, unsafe extern "C" fn() -> CQuery>,
    // c_when_handler: libloading::Symbol<'a, unsafe extern "C" fn(unsafe extern "C" fn(Tuple) -> Tuple) -> ()>,
}

impl CWhen {

    pub fn new<'a>(libPath: &str) -> Result<CWhen> {
        let lib = unsafe { libloading::Library::new(libPath).unwrap() };

        Ok(CWhen { lib })
    }
}

impl When for CWhen {
    fn get_query(&self) -> Query {
        let c_get_query: libloading::Symbol<unsafe extern "C" fn() -> CQuery> =
            unsafe { self.lib.get(b"get_query").unwrap() };

        // Parens necessary, apparently.
        let cq = unsafe { c_get_query() };

        Query {
            subject: cq.subject.map(TupleNoun::CPtr),
            predicate: cq.predicate.map(|s| s.into_string().unwrap()),
            object: cq.object.map(TupleNoun::CPtr),
        }
    }

    fn handle(&mut self, wish: &mut dyn FnMut(Tuple), results: Tuple) -> () {
        let c_when_handler: libloading::Symbol<unsafe extern "C" fn(unsafe extern "C" fn(Tuple) -> Tuple) -> ()> =
            unsafe { self.lib.get(b"when_handler").unwrap() };

        todo!()
    }
}