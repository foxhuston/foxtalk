use crate::reactor::reactor_handler::Handler;
use crate::triples_reactor::serde::*;
use crate::triples_reactor::Tuple;
use libloading::os::unix::{Library, Symbol, RTLD_LOCAL, RTLD_NOW};
use std::collections::HashSet;
use std::path::Path;
use std::slice;

#[derive(Debug)]
pub struct DynamicHandler {
    _lib: Library,
    _init: Symbol<extern "C" fn() -> ()>,
    free_tuple: Symbol<extern "C" fn() -> ()>,
    matches: Symbol<extern "C" fn() -> bool>,
    handle: Symbol<extern "C" fn() -> ()>,
    teardown: Symbol<extern "C" fn() -> ()>,
    buffer: Symbol<*mut u8>,
}

unsafe impl Send for DynamicHandler{}

impl DynamicHandler {
    pub fn get_bootstrap_output(&self) -> HashSet<Tuple> {
        let buffer = unsafe { slice::from_raw_parts_mut(*self.buffer, 10_000_000) };
        let (res, _) = Vec::<Tuple>::read_from_buffer(buffer, 0);
        HashSet::from_iter(res)
    }
    pub unsafe fn new(path: &Path) -> Self {
        // Magic 0x08 number is DEEPBIND for dlopen.
        let lib = Library::open(Some(path), RTLD_NOW | RTLD_LOCAL | 0x08).unwrap();

        let init: Symbol<extern "C" fn() -> ()> = lib.get(b"init").unwrap();
        let free_tuple: Symbol<extern "C" fn() -> ()> = lib.get(b"free_tuple").unwrap();
        let matches: Symbol<extern "C" fn() -> bool> = lib.get(b"matches").unwrap();
        let handle: Symbol<extern "C" fn() -> ()> = lib.get(b"handle").unwrap();
        let teardown: Symbol<extern "C" fn() -> ()> = lib.get(b"teardown").unwrap();
        let buffer: Symbol<*mut u8> = lib.get(b"_foxtalk_ipc_buffer").unwrap();

        init();

        DynamicHandler {
            _lib: lib,
            _init: init,
            free_tuple,
            matches,
            handle,
            teardown,
            buffer,
        }
    }
}

impl Drop for DynamicHandler {
    fn drop(&mut self) {
        (self.teardown)();
    }
}

impl Handler<Tuple> for DynamicHandler {
    fn query(&mut self, o: &Tuple) -> bool {
        let buffer = unsafe { slice::from_raw_parts_mut(*self.buffer, 10_000_000) };
        o.write_to_buffer(buffer, 0);
        (&self.matches)()
    }

    fn handle(&mut self, input: &HashSet<Tuple>) -> HashSet<Tuple> {
        let buffer = unsafe { slice::from_raw_parts_mut(*self.buffer, 10_000_000) };
        input.into_iter().write_to_buffer(buffer, 0);
        (self.handle)();
        let (res, _) = Vec::<Tuple>::read_from_buffer(buffer, 0);
        HashSet::from_iter(res)
    }
    fn free_o(&mut self, o: &Tuple) -> () {
        let buffer = unsafe { slice::from_raw_parts_mut(*self.buffer, 10_000_000) };
        o.write_to_buffer(buffer, 0);
        (&self.free_tuple)()
    }
}