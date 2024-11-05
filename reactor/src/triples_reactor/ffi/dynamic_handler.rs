
use anyhow::{Result, format_err};

use crate::triples_reactor::Tuple;
use libloading::os::unix::{Library, Symbol, RTLD_LOCAL, RTLD_NOW};
use std::path::Path;
use std::slice;
use rust_tuple_reactor_serde::{FoxTalkDeserializable, FoxTalkOwnedSerializable, FoxTalkSerializable};
use rustc_hash::FxHashSet;
use crate::reactor::reactor_program::Program;

#[derive(Debug)]
pub struct DynamicallyLoadedProgram {
    _lib: Library,
    _init: Symbol<extern "C" fn() -> ()>,
    free_tuple: Symbol<extern "C" fn() -> ()>,
    _register_initial_tuples: Symbol<extern "C" fn() -> ()>,
    handle: Symbol<extern "C" fn() -> ()>,
    teardown: Symbol<extern "C" fn() -> ()>,
    buffer: Symbol<*mut u8>,
    poll: Symbol<extern "C" fn() -> bool>,
    query: Vec<Tuple>
}

unsafe impl Send for DynamicallyLoadedProgram {}

static BUFFER_SIZE: usize = 10 * 1024 * 1024;

impl DynamicallyLoadedProgram {
    pub fn get_bootstrap_output(&self) -> FxHashSet<Tuple> {
        let buffer = unsafe { slice::from_raw_parts_mut(*self.buffer, BUFFER_SIZE) };
        let (res, _) = Vec::<Tuple>::read_from_buffer(buffer, 0);
        FxHashSet::from_iter(res)
    }
    pub unsafe fn new(path: &Path) -> Result<Self> {
        // Magic 0x08 number is DEEPBIND for dlopen.
        let lib = Library::open(Some(path), RTLD_NOW | RTLD_LOCAL | 0x08)?;

        let init: Symbol<extern "C" fn() -> ()> = lib.get(b"init")?;
        let free_tuple: Symbol<extern "C" fn() -> ()> = lib.get(b"free_tuple")?;
        let register_initial_tuples: Symbol<extern "C" fn() -> ()> = lib.get(b"register_initial_tuples")?;
        let handle: Symbol<extern "C" fn() -> ()> = lib.get(b"handle")?;
        let teardown: Symbol<extern "C" fn() -> ()> = lib.get(b"teardown")?;
        let poll: Symbol<extern "C" fn() -> bool> = lib.get(b"poll")?;
        let buffer: Symbol<*mut u8> = lib.get(b"_foxtalk_ipc_buffer")?;

        init();
        let rust_buffer = unsafe { slice::from_raw_parts_mut(*buffer, BUFFER_SIZE) };
        let (query, _) = Vec::<Tuple>::read_from_buffer(rust_buffer, 0);

        if query.is_empty() {
            return Err(format_err!("Query not loaded into the buffer after calling init on {:?}. Not loading program.", path));
        }

        register_initial_tuples();

        Ok(DynamicallyLoadedProgram {
            _lib: lib,
            _init: init,
            free_tuple,
            _register_initial_tuples: register_initial_tuples,
            handle,
            teardown,
            buffer,
            poll,
            query
        })
    }
}

impl Drop for DynamicallyLoadedProgram {
    fn drop(&mut self) {
        (self.teardown)();
    }
}

impl Program<Tuple, Vec<Tuple>> for DynamicallyLoadedProgram {
    fn query(&mut self) -> Vec<Tuple> {
        self.query.clone()
    }

    fn handle(&mut self, input: &FxHashSet<Tuple>) -> FxHashSet<Tuple> {
        let buffer = unsafe { slice::from_raw_parts_mut(*self.buffer, BUFFER_SIZE) };
        input.into_iter().write_to_buffer(buffer, 0);
        (self.handle)();
        let (res, _) = Vec::<Tuple>::read_from_buffer(buffer, 0);
        FxHashSet::from_iter(res)
    }
    fn poll(&mut self) -> bool {
        (self.poll)()
    }
    fn free_o(&mut self, o: &Tuple) -> () {
        let buffer = unsafe { slice::from_raw_parts_mut(*self.buffer, BUFFER_SIZE) };
        o.write_to_buffer(buffer, 0);
        (&self.free_tuple)()
    }
}