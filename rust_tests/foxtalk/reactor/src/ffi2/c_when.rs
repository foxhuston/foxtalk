use std::ffi::c_void;
use std::mem;
use std::ptr::NonNull;

use anyhow::Result;

use libloading::os::unix::{Library, Symbol};
use crate::ffi2::{get_c_tuple, PtrTuple};
use crate::tuple::Tuple;
use crate::when::When;

type CGetQuery = unsafe extern "C" fn() -> *mut PtrTuple;
type CWhenHandler = unsafe extern "C" fn(*mut PtrTuple, *mut usize) -> *mut *mut PtrTuple;

// TODO: I could make this much nicer by having a pointer to a specific provenance tuple as a
//   global symbol that each individual CWhen could set when it called its handler. I could probably
//   set up provenance while still having functions like `claim` and `wish`, which is important
//   if I ever want to allow programs to `remove` tuples from the db (important for e.g. the
//   loader that watches for paper program IDs to appear and disappear on/from the table).
pub struct CWhen {
    lib: Library,
    get_query: Symbol<CGetQuery>,
    when_handler: Symbol<CWhenHandler>,
}

impl CWhen {
    pub unsafe fn new(so_path: &str) -> Result<Self> {
        let lib = Library::new(so_path)?;

        let get_query: Symbol<CGetQuery> = lib.get(b"get_query")?;
        let when_handler: Symbol<CWhenHandler> = lib.get(b"when_handler")?;

        Ok(CWhen {
            lib,
            get_query,
            when_handler,
        })
    }
}

impl When for CWhen {
    fn get_query(&self) -> Tuple {
        let cq = unsafe { (&self.get_query)() };
        cq.into()
    }

    fn handle(&mut self, results: Tuple) -> Vec<Tuple> {
        println!("C <--> Rust handler begin for:");
        println!("    RUST tuple {results:?}");

        let query_result_tuple = results.into();

        let mut out_size: usize = 0;
        let ctuple_array = unsafe { (&self.when_handler)(query_result_tuple, &mut out_size) };

        if ctuple_array.is_null() { return vec![]; }

        let ctuples = unsafe { Vec::from_raw_parts(ctuple_array, out_size, out_size) };

        let out = ctuples.into_iter().map(|ptr_tuple| { ptr_tuple.into() }).collect();

        println!("C <--> Rust handler end");

        out
    }
}
