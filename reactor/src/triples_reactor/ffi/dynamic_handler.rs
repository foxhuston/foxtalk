use crate::triples_reactor::serde::*;
use crate::triples_reactor::Tuple;
use libloading::os::unix::{Library, Symbol, RTLD_LOCAL, RTLD_NOW};
use std::path::Path;
use std::slice;
use rustc_hash::FxHashSet;
use crate::reactor::reactor_program::Program;

#[derive(Debug)]
pub struct DynamicallyLoadedProgram {
    _lib: Library,
    _init: Symbol<extern "C" fn() -> ()>,
    free_tuple: Symbol<extern "C" fn() -> ()>,
    register_initial_tuples: Symbol<extern "C" fn() -> ()>,
    handle: Symbol<extern "C" fn() -> ()>,
    teardown: Symbol<extern "C" fn() -> ()>,
    buffer: Symbol<*mut u8>,
    query: Tuple
}

unsafe impl Send for DynamicallyLoadedProgram {}

impl DynamicallyLoadedProgram {
    pub fn get_bootstrap_output(&self) -> FxHashSet<Tuple> {
        let buffer = unsafe { slice::from_raw_parts_mut(*self.buffer, 10_000_000) };
        let (res, _) = Vec::<Tuple>::read_from_buffer(buffer, 0);
        FxHashSet::from_iter(res)
    }
    pub unsafe fn new(path: &Path) -> Self {
        // Magic 0x08 number is DEEPBIND for dlopen.
        let lib = Library::open(Some(path), RTLD_NOW | RTLD_LOCAL | 0x08).unwrap();

        let init: Symbol<extern "C" fn() -> ()> = lib.get(b"init").unwrap();
        let free_tuple: Symbol<extern "C" fn() -> ()> = lib.get(b"free_tuple").unwrap();
        let register_initial_tuples: Symbol<extern "C" fn() -> ()> = lib.get(b"register_initial_tuples").unwrap();
        let handle: Symbol<extern "C" fn() -> ()> = lib.get(b"handle").unwrap();
        let teardown: Symbol<extern "C" fn() -> ()> = lib.get(b"teardown").unwrap();
        let buffer: Symbol<*mut u8> = lib.get(b"_foxtalk_ipc_buffer").unwrap();

        init();
        let rust_buffer = unsafe { slice::from_raw_parts_mut(*buffer, 10_000_000) };
        let tuples: Vec<Tuple> = Vec::<Tuple>::read_from_buffer(rust_buffer, 0).0;
        let query_expect_msg = format!("Query not loaded into the buffer after calling init on {:?}", path);
        let query = tuples.first().expect(query_expect_msg.as_str()).to_owned();
        println!("Got query: {:?}", &query); 
        register_initial_tuples();

        DynamicallyLoadedProgram {
            _lib: lib,
            _init: init,
            free_tuple,
            register_initial_tuples,
            handle,
            teardown,
            buffer,
            query
        }
    }
}

impl Drop for DynamicallyLoadedProgram {
    fn drop(&mut self) {
        (self.teardown)();
    }
}

impl Program<Tuple, Tuple> for DynamicallyLoadedProgram {
    fn query(&mut self) -> Tuple {
        self.query.clone()
    }

    fn handle(&mut self, input: &FxHashSet<Tuple>) -> FxHashSet<Tuple> {
        let buffer = unsafe { slice::from_raw_parts_mut(*self.buffer, 10_000_000) };
        input.into_iter().write_to_buffer(buffer, 0);
        (self.handle)();
        let (res, _) = Vec::<Tuple>::read_from_buffer(buffer, 0);
        FxHashSet::from_iter(res)
    }
    fn free_o(&mut self, o: &Tuple) -> () {
        let buffer = unsafe { slice::from_raw_parts_mut(*self.buffer, 10_000_000) };
        o.write_to_buffer(buffer, 0);
        (&self.free_tuple)()
    }
}