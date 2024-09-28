use std::cell::{OnceCell, RefCell};
use std::ffi::{c_void, CStr, CString};
use std::ptr::NonNull;
use std::sync::Arc;
use libc::c_char;
use crate::query::Query;
use crate::tuple::{Tuple, TupleNoun};

use anyhow::{format_err, Result};

use libloading::os::unix::{Library, Symbol};

use crate::when::When;

#[repr(C)]
#[derive(Debug)]
pub struct CTuple {
    pub subject: *mut c_void,
    pub predicate: *mut c_char,
    pub object: *mut c_void,
}

impl Tuple {
    fn from_ctuple(free_subj: NonNull<c_void>, free_obj: NonNull<c_void>, value: CTuple) -> Self {
        let s = NonNull::new(value.subject).map(|s| {
            TupleNoun::CPtrHeap { data: s, destructor: free_subj }
        }).unwrap();

        let p = if value.predicate.is_null() { None } else { Some(unsafe { CStr::from_ptr(value.predicate) }) };
        let p = p.map(|s| s.to_str().unwrap().to_owned()).unwrap();

        let o = NonNull::new(value.object).map(|s| {
            TupleNoun::CPtrHeap { data: s, destructor: free_obj }
        }).unwrap();

        Tuple {
            subject: s,
            predicate: p,
            object: o
        }
    }
}

impl From<TupleNoun> for *mut c_void {
    fn from(value: TupleNoun) -> Self {
        match &value {
            TupleNoun::CPtr(p) => { p.as_ptr() }
            TupleNoun::CPtrHeap { data, destructor} => { data.as_ptr() }
            TupleNoun::Str(s) => {
                CString::new(s.clone()).unwrap().into_raw().cast()
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
        let p = if value.predicate.is_null() { None } else { Some(unsafe { CStr::from_ptr(value.predicate) }) };
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

type CWhenHandler = unsafe extern "C" fn(*const CTuple, *mut usize) -> *mut CTuple;
type CGetQuery = unsafe extern "C" fn(&mut CTuple) -> ();
type CFreeTuple = unsafe extern "C" fn(&mut CTuple) -> ();

////////////////////////////////////////////////////////////////////////////////


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
        let mut cq = CTuple::default();
        unsafe { (&self.get_query)(&mut cq) };

        println!("DEBUG! Got CQuery {cq:?}");

        cq.into()
    }

    fn handle(&mut self, results: Tuple) -> Vec<Tuple> {
        let t: CTuple = results.into();

        let mut outSize: usize = 0;
        let ctupleArray = unsafe { (&self.when_handler)(&t, &mut outSize) };

        if ctupleArray.is_null() { return vec![]; }

        let ctuples = unsafe { Vec::from_raw_parts(ctupleArray, outSize, outSize) };

        ctuples.into_iter().map(|tuple| {
            Tuple::from_ctuple(self.free_tuple_subj, self.free_tuple_obj, tuple)
        }).collect()
    }
}