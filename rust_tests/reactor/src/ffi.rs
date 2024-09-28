use crate::query::Query;
use crate::tuple::Tuple;
use crate::when::When;
use std::collections::HashMap;
use std::ffi::{c_void, CString};

use crate::tuple::TupleNoun;
use anyhow::Result;

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

type CWhenHandler = unsafe extern "C" fn(unsafe extern "C" fn(Tuple) -> Tuple) -> ();
type CGetQuery = unsafe extern "C" fn() -> CQuery;

type Library<'a> = (&'a libloading::Library, libloading::Symbol<'a, CGetQuery>, libloading::Symbol<'a, CWhenHandler>);

struct LibraryRegistry<'a> {
    lib_map: HashMap<&'static str, libloading::Library>,
    libraries: HashMap<&'static str, Library<'a>>,
}

impl<'b> LibraryRegistry<'b> {

    // Just a small wrapper around CWhen::new
    pub fn cwhen_for<'a>(&'b mut self, lib_path: &'static str) -> Result<CWhen<'a, 'b>> {
        CWhen::new(lib_path, self)
    }

    pub fn new() -> Self {
        LibraryRegistry {
            libraries: HashMap::new(),
            lib_map: HashMap::new(),
        }
    }

    pub fn get<'a>(&'b mut self, lib_path: &'static str) -> &'a mut Library<'b>
    where 'a: 'b {

        self.libraries.entry(lib_path).or_insert_with(|| {

            self.lib_map.insert(lib_path, unsafe { libloading::Library::new(lib_path).unwrap() });

            let c_get_query: libloading::Symbol<'a, CGetQuery> = unsafe { self.lib_map.get(lib_path).unwrap().get(b"get_query").unwrap() };
            let c_when_handler: libloading::Symbol<'a, CWhenHandler> = unsafe { self.lib_map.get(lib_path).unwrap().get(b"when_handler").unwrap() };

            (self.lib_map.get(lib_path).unwrap(), c_get_query, c_when_handler)
        })
    }

}

struct CWhen<'a, 'b> where 'a: 'b {
    lib: &'a libloading::Library,
    c_get_query: &'a libloading::Symbol<'b, CGetQuery>,
    c_when_handler: &'a libloading::Symbol<'b, CWhenHandler>
}

impl<'a, 'b> CWhen<'a, 'b> {
    pub fn new(lib_path: &'static str, library_registry: &'a mut LibraryRegistry<'b>) -> Result<CWhen<'a, 'b>> where 'a: 'b {
        let library = library_registry.get(lib_path);

        Ok(CWhen {
            lib: &library.0,
            c_get_query: &library.1,
            c_when_handler: &library.2,
        })
    }

}

impl<'a, 'b> When for CWhen<'a, 'b> {
    fn get_query(&self) -> Query {
        // Parens necessary, apparently.
        let cq = unsafe { (&self.c_get_query)() };

        Query {
            subject: cq.subject.map(TupleNoun::CPtr),
            predicate: cq.predicate.map(|s| s.into_string().unwrap()),
            object: cq.object.map(TupleNoun::CPtr),
        }
    }

    fn handle(&mut self, wish: &mut dyn FnMut(Tuple), results: Tuple) -> () {
        todo!()
    }
}