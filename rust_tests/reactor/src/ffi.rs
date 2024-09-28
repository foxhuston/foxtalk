use std::arch::x86_64::_subborrow_u64;
use crate::query::Query;
use crate::tuple::Tuple;
use crate::when::When;
use std::collections::HashMap;
use std::ffi::{c_void, CStr, CString};
use std::ptr::NonNull;
use crate::tuple::TupleNoun;
use anyhow::Result;
use libc::{c_char, CS};

#[repr(C)]
#[derive(Debug)]
struct CTuple {
    pub subject: *mut c_void,
    pub predicate: *mut c_char,
    pub object: *mut c_void,
}

impl From<CTuple> for Tuple {
    fn from(value: CTuple) -> Self {
        let s = NonNull::new(value.subject).map(TupleNoun::CPtr).unwrap();
        let p = if(value.predicate.is_null()) { None } else { Some(unsafe { CStr::from_ptr(value.predicate) }) };
        let p = p.map(|s| s.to_str().unwrap().to_owned()).unwrap();
        let o = NonNull::new(value.object).map(TupleNoun::CPtr).unwrap();

        Tuple {
            subject: s,
            predicate: p,
            object: o
        }
    }
}


impl From<TupleNoun> for *mut c_void {
    fn from(value: TupleNoun) -> Self {
        match value {
            TupleNoun::CPtr(p) => { p.as_ptr() }
            TupleNoun::Str(s) => {
                CString::new(s).unwrap().into_raw().cast()
            }
        }
    }
}

impl From<Tuple> for CTuple {
    fn from(value: Tuple) -> Self {

        CTuple {
            subject: value.subject.into(),
            predicate: CString::new(value.predicate).unwrap().into_raw(),
            object: value.object.into()
        }
    }
}

impl From<CTuple> for Query {
    fn from(value: CTuple) -> Self {
        let s = NonNull::new(value.subject).map(TupleNoun::CPtr);
        let p = if(value.predicate.is_null()) { None } else { Some(unsafe { CStr::from_ptr(value.predicate) }) };
        let p = p.map(|s| s.to_str().unwrap().to_owned());
        let o = NonNull::new(value.object).map(TupleNoun::CPtr);

        Query {
            subject: s,
            predicate: p,
            object: o,
        }
    }
}


impl Default for CTuple {
    fn default() -> Self {
        CTuple { subject: std::ptr::null_mut(), predicate: std::ptr::null_mut(), object: std::ptr::null_mut() }
    }
}

#[no_mangle]
extern "C" fn wish(tuple: Tuple) {
    println!("{:?}", tuple);
}

// type CWhenHandler = unsafe extern "C" fn(unsafe extern "C" fn(Tuple) -> (), Tuple) -> ();
type CWhenHandler = unsafe extern "C" fn(CTuple) -> ();
type CGetQuery = unsafe extern "C" fn(&mut CTuple) -> ();

pub type Library<'a> = (&'a libloading::Library, libloading::Symbol<'a, CGetQuery>, libloading::Symbol<'a, CWhenHandler>);

pub struct LibraryRegistry<'a> {
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

pub struct CWhen<'a, 'b> where 'a: 'b {
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

    pub fn handle_DEBUG(&mut self, results: Tuple) -> () {
        unsafe { (&self.c_when_handler)(results.into()) };
    }
}

impl<'a, 'b> When for CWhen<'a, 'b> {
    fn get_query(&self) -> Query {
        // Parens necessary, apparently.
        let mut cq = CTuple::default();
        unsafe { (&self.c_get_query)(&mut cq) };

        println!("DEBUG! Got CQuery {cq:?}");

        cq.into()
    }

    fn handle(&mut self, wish: &mut dyn FnMut(Tuple), results: Tuple) -> () {
        unsafe { (&self.c_when_handler)(results.into()) };
    }
}