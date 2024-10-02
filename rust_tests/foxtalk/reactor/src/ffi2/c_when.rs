use anyhow::Result;
use std::sync::Arc;

use crate::ffi2::PtrTuple;
use crate::when::When;
use libloading::os::unix::{Library, Symbol};
use tuple_db::tuple::Tuple;

type CGetQuery = unsafe extern "C" fn() -> *mut PtrTuple;
type CWhenHandler = unsafe extern "C" fn(*mut PtrTuple, *mut usize) -> *mut *mut PtrTuple;

// TODO: I could make this much nicer by having a pointer to a specific provenance tuple as a
//   global symbol that each individual CWhen could set when it called its handler. I could probably
//   set up provenance while still having functions like `claim` and `wish`, which is important
//   if I ever want to allow programs to `remove` tuples from the db (important for e.g. the
//   loader that watches for paper program IDs to appear and disappear on/from the table).
#[allow(dead_code)]
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
    fn get_query(&self) -> Arc<Tuple> {
        let cq = unsafe { (&self.get_query)() };
        match unsafe { cq.as_ref() } {
            Some(cq) => cq.to_tuple(),
            None => panic!("C get_query returned null")
        }
    }

    fn handle(&mut self, results: Arc<Tuple>) -> Vec<Arc<Tuple>> {
        println!("C <--> Rust handler begin for:");
        println!("    RUST tuple {results:?}");

        let query_result_tuple = PtrTuple::from_tuple(results);

        let mut out_size: usize = 0;
        let ctuple_array = unsafe { (&self.when_handler)(query_result_tuple, &mut out_size) };

        if ctuple_array.is_null() { return vec![]; }

        let ctuples = unsafe { Vec::from_raw_parts(ctuple_array, out_size, out_size) };

        let out = ctuples
            .iter()
            .map(|ptr_tuple| {
                PtrTuple::to_tuple(unsafe { ptr_tuple.as_ref().unwrap() })
            })
            .collect();

        println!("C <--> Rust handler end");

        out
    }
}
