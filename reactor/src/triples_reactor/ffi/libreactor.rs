use std::ffi::{c_void, CStr, CString};
use crate::reactor::Reactor;
use crate::triples_reactor::ffi::HandlerRegistry;
use crate::triples_reactor::Tuple;

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
    // (*reactor_ptr).reactor.insert();
}

#[no_mangle]
pub extern "C" fn remove_tuple(reactor_ptr: *mut c_void, buff: &[u8]) { }

#[no_mangle]
pub extern "C" fn add_handler(reactor_ptr: *mut c_void, path: &CStr) { }

#[no_mangle]
pub extern "C" fn remove_handler(reactor_ptr: *mut c_void, path: &CStr) { }

#[no_mangle]
pub extern "C" fn tick(reactor: *mut c_void) { }



















