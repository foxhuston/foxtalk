use std::collections::HashMap;
use std::path::PathBuf;

use std::ffi::c_char;

use libloading;
use crate::triples_reactor::serde::FoxTalkSerializable;
use crate::triples_reactor::Tuple;
use crate::utils::ReactorHandler;
/*
    This is the extern c api:
    
    
extern "C" {
/**
 * This is the runtime communication buffer, fixed to 10Mb by the constant above.
 * The first `sizeof(foxtalk_size_t)` bytes are the number of tuples in the buffer, which
 * are NOT GUARANTEED TO BE UNIQUE! After that is the standard serialization of Tuples,
 * which can be seen in `foxtalk_tuple.h`.
 */
inline uint8_t _foxtalk_ipc_buffer[FOXTALK_IPC_BUFFER_SIZE];

///// USER MUST IMPLEMENT /////

// from the <q, a, S, I, O> in the paper...
void init();
void free_tuple_nouns();
bool matches(); // this is q
void handle();  // this is a
void teardown();
}

 */

pub struct FoxTalkHandlerLib<'a> {
    init: libloading::Symbol<'a, extern "C" fn() -> ()>,
    free_tuple_nouns: libloading::Symbol<'a, extern "C" fn() -> ()>,
    matches: libloading::Symbol<'a, extern "C" fn() -> bool>,
    handle: libloading::Symbol<'a, extern "C" fn() -> ()>,
    teardown: libloading::Symbol<'a, extern "C" fn() -> ()>,
    buffer: &'a mut [u8]
}
impl<'a> FoxTalkHandlerLib<'a> {
    fn query(&mut self, tuple_to_match: &Tuple) -> bool {
        tuple_to_match.write_to_buffer(self.buffer, 0);
        (*self.matches)()
    }
}
impl<'a> Into<ReactorHandler<Tuple>> for FoxTalkHandlerLib<'a> {
    fn into(mut self) -> ReactorHandler<Tuple> {
        
        todo!()
        // ReactorHandler::new(|o| { self.query(o) }, todo!())
    }
}

pub struct HandlerRegistry<'a> {
    handlers: HashMap<String, FoxTalkHandlerLib<'a>>
}

impl<'a> HandlerRegistry<'a> {
    pub fn new() -> Self {
        HandlerRegistry {
            handlers: HashMap::new()
        }
    }
    
    // pub fn load_handler(&mut self, path_buf: PathBuf) {
    // 
    // 
    //     let lib = unsafe { libloading::Library::new(path_buf).unwrap() };
    //     let init: libloading::Symbol<extern "C" fn() -> ()> = unsafe { lib.get(b"init").unwrap() };
    //     let free_tuple_nouns: libloading::Symbol<extern "C" fn() -> ()> = unsafe { lib.get(b"free_tuple_nouns").unwrap() };
    //     let matches: libloading::Symbol<extern "C" fn() -> bool> = unsafe { lib.get(b"matches").unwrap() };
    //     let handle: libloading::Symbol<extern "C" fn() -> ()> = unsafe { lib.get(b"handle").unwrap() };
    //     let teardown: libloading::Symbol<extern "C" fn() -> ()> = unsafe { lib.get(b"teardown").unwrap() };
    // 
    //     let buffer: libloading::Symbol<*mut c_char> = unsafe { lib.get(b"_foxtalk_ipc_buffer").unwrap() };
    // 
    //     let t = FoxTalkHandlerLib {
    //         init,
    //         free_tuple_nouns,
    //         matches,
    //         handle,
    //         teardown,
    //         buffer: unsafe { std::slice::from_raw_parts_mut(*buffer as *mut u8, 10_000_000) }
    //     };
    //     
    // 
    //     self.handlers.insert("".to_string(), t);
    // }
    
}
