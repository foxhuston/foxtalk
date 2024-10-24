use std::ffi::{c_void, CStr, CString};
use std::path::*;
use crate::reactor::Reactor;
use crate::triples_reactor::ffi::DynamicHandler;
use crate::triples_reactor::Tuple;
use crate::triples_reactor::serde::*;

pub struct DynamicLibFfiReactor {
    reactor: Reactor<Tuple>
}

impl DynamicLibFfiReactor {
    pub fn new() -> Self {
        DynamicLibFfiReactor {
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
    let dload_handler = DynamicHandler::new(path);
    (*reactor_ptr).reactor.add_handler(Box::new(dload_handler))
}

#[no_mangle]
pub unsafe extern "C" fn remove_handler(reactor_ptr: *mut DynamicLibFfiReactor, path: &CStr) { }

#[no_mangle]
pub unsafe extern "C" fn tick(reactor_ptr: *mut DynamicLibFfiReactor) {
    (*reactor_ptr).reactor.tick();
}



















