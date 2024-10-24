use crate::triples_reactor::serde::{parse_tuples, FoxTalkSerializable};
use crate::triples_reactor::Tuple;
use crate::utils::ReactorHandle;
use libloading;
use std::collections::{HashMap, HashSet};
use std::ffi::c_char;
use std::path::PathBuf;

pub struct FoxTalkHandlerLib<'a> {
    pub init: libloading::Symbol<'a, extern "C" fn() -> ()>,
    pub free_tuple_nouns: libloading::Symbol<'a, extern "C" fn() -> ()>,
    pub matches: libloading::Symbol<'a, extern "C" fn() -> bool>,
    pub handle: libloading::Symbol<'a, extern "C" fn() -> ()>,
    pub teardown: libloading::Symbol<'a, extern "C" fn() -> ()>,
    pub buffer: &'a mut [u8]
}

// impl<'a> Drop for FoxTalkHandlerLib<'a> {
//     fn drop(&mut self) {
//         todo!()
//     }
// }
impl<'a> ReactorHandle<Tuple> for FoxTalkHandlerLib<'a> {
    fn q(&mut self, o: &Tuple) -> bool {
        o.write_to_buffer(self.buffer, 0);
        (self.matches)()
    }

    // This assumes this is always aggregating handlers right now
    fn a(&mut self, o: &HashSet<&Tuple>) -> HashSet<Tuple> {
        let to_write: Vec<&Tuple> = o.iter().map(|x| *x).collect();
        to_write.write_to_buffer(self.buffer, 0);
        (self.handle)();
        let res = parse_tuples(self.buffer);
        HashSet::from_iter(res)
    }
}

// Okay, so, I think adding the lifetime here is going to mean the handlers can't be freed during
// the runtime.
// TODO: Check this with a memory profiler
pub struct HandlerRegistry<'a> {
    pub libs: HashMap<String, libloading::Library>,
    pub handlers: HashMap<String, &'a FoxTalkHandlerLib<'a>>
}

impl<'a> HandlerRegistry<'a> {
    pub fn new() -> Self {
        HandlerRegistry {
            libs: HashMap::new(),
            handlers: HashMap::new()
        }
    }

    pub fn create_handler(&'a mut self, path_buf: PathBuf) -> FoxTalkHandlerLib<'a> {

        let name = path_buf.iter().last().unwrap().to_str().unwrap().to_string();

        unsafe { self.libs.insert(name.clone(), libloading::Library::new(path_buf).unwrap()) };
        let lib = self.libs.get(&name).unwrap();
        let init: libloading::Symbol<extern "C" fn() -> ()> = unsafe { lib.get(b"init").unwrap() };
        let free_tuple_nouns: libloading::Symbol<extern "C" fn() -> ()> = unsafe { lib.get(b"free_tuple_nouns").unwrap() };
        let matches: libloading::Symbol<extern "C" fn() -> bool> = unsafe { lib.get(b"matches").unwrap() };
        let handle: libloading::Symbol<extern "C" fn() -> ()> = unsafe { lib.get(b"handle").unwrap() };
        let teardown: libloading::Symbol<extern "C" fn() -> ()> = unsafe { lib.get(b"teardown").unwrap() };

        let buffer: libloading::Symbol<*mut c_char> = unsafe { lib.get(b"_foxtalk_ipc_buffer").unwrap() };

        let t = FoxTalkHandlerLib {
            init,
            free_tuple_nouns,
            matches,
            handle,
            teardown,
            buffer: unsafe { std::slice::from_raw_parts_mut(*buffer as *mut u8, 10_000_000) }
        };

        t
    }
    
}
