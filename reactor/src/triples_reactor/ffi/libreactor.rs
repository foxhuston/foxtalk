use std::ffi::{c_void, CStr, CString};
use std::path::*;
use crate::reactor::Reactor;
use crate::triples_reactor::ffi::HandlerRegistry;
use crate::triples_reactor::Tuple;
use crate::triples_reactor::serde::*;

struct DynamicLibFfiReactor<'a> {
    handler_registry: HandlerRegistry<'a>,
    reactor: Reactor<'a, Tuple>
}

impl<'a> DynamicLibFfiReactor<'a> {
    pub fn new() -> Self {
        DynamicLibFfiReactor {
            handler_registry: HandlerRegistry::new(),
            reactor: Reactor::new()
        }
    }
}

#[no_mangle]
pub extern "C" fn mk_reactor() -> *mut c_void {
    Box::into_raw(Box::new(DynamicLibFfiReactor::new())).cast()
}

#[no_mangle]
pub unsafe extern "C" fn free_reactor(reactor: *mut c_void) -> () {
    let _: Box<DynamicLibFfiReactor> = unsafe {
        Box::from_raw(reactor.cast())
    };
}

#[no_mangle]
pub unsafe extern "C" fn add_tuple(reactor_ptr: *mut DynamicLibFfiReactor, buff: &[u8]) {
    let (t, _) = Tuple::read_from_buffer(buff, 0);
    (*reactor_ptr).reactor.insert(t);
}

#[no_mangle]
pub unsafe extern "C" fn remove_tuple(reactor_ptr: *mut DynamicLibFfiReactor, buff: &[u8]) {
    let (t, _) = Tuple::read_from_buffer(buff, 0);
    (*reactor_ptr).reactor.remove(t);
}

#[no_mangle]
pub unsafe extern "C" fn add_handler(reactor_ptr: *mut DynamicLibFfiReactor, path: &CStr) {
    let path = Path::new(path.to_str().unwrap());
    let x = (*reactor_ptr).handler_registry.create_handler(path);
    (*reactor_ptr).handler_registry.handlers(x)
}

#[no_mangle]
pub unsafe extern "C" fn remove_handler(reactor_ptr: *mut DynamicLibFfiReactor, path: &CStr) { }

#[no_mangle]
pub unsafe extern "C" fn tick(reactor: *mut DynamicLibFfiReactor) { }



















