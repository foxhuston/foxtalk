pub mod libreactor;

use std::collections::HashSet;
use std::ffi::c_char;
use std::path::Path;
use libloading::os::unix::{Library, Symbol};
use crate::reactor::reactor_handler::Handler;
use crate::triples_reactor::serde::*;
use crate::triples_reactor::Tuple;

pub struct DynamicHandler<'a> {
    lib: Library,
    init: Symbol<extern "C" fn() -> ()>,
    free_tuple_nouns: Symbol<extern "C" fn() -> ()>,
    matches: Symbol<extern "C" fn() -> bool>,
    handle: Symbol<extern "C" fn() -> ()>,
    teardown: Symbol<extern "C" fn() -> ()>,
    buffer: &'a mut [u8],
}

impl DynamicHandler<'_> {
    pub unsafe fn new(path: &Path) -> Self {
        let lib = Library::new(path).unwrap();

        let init: Symbol<extern "C" fn() -> ()> = lib.get(b"init").unwrap();
        let free_tuple_nouns: Symbol<extern "C" fn() -> ()> = lib.get(b"free_tuple_nouns").unwrap();
        let matches: Symbol<extern "C" fn() -> bool> = lib.get(b"matches").unwrap();
        let handle: Symbol<extern "C" fn() -> ()> = lib.get(b"handle").unwrap();
        let teardown: Symbol<extern "C" fn() -> ()> = lib.get(b"teardown").unwrap();
        let buffer: Symbol<*mut c_char> = lib.get(b"_foxtalk_ipc_buffer").unwrap();

        init();
        DynamicHandler {
            lib, init, free_tuple_nouns, matches, handle, teardown,
            buffer: unsafe { std::slice::from_raw_parts_mut(*buffer as *mut u8, 10_000_000) }
        }
    }
}

impl Handler<Tuple> for DynamicHandler<'_> {
    fn query(&mut self, o: &Tuple) -> bool {
        o.write_to_buffer(self.buffer, 0);
        (&self.matches)()
    }

    fn handle(&mut self, input: &HashSet<Tuple>) -> HashSet<Tuple> {
        input.into_iter().write_to_buffer(self.buffer, 0);
        (self.handle)();
        let (res, _) = Vec::<Tuple>::read_from_buffer(self.buffer, 0);
        HashSet::from_iter(res)
    }
}

impl Drop for DynamicHandler<'_> {
    fn drop(&mut self) {
        println!("Dropping DynamicHandler");
    }
}

#[cfg(test)]
pub mod test {
    use super::*;

    use std::path::PathBuf;
    use crate::reactor::Reactor;
    use crate::triples_reactor::TupleNoun;

    fn linked_lib_path(filename: &str) -> PathBuf {
        let mut path = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
        path.push("tests/test_libs/out");
        path.push(filename);
        let new_path = path.clone();
        let path_str = new_path.to_str().unwrap();
        let owned_path = path_str;
        PathBuf::from(owned_path)
    }

    fn add_handler_to_reactor(reactor: &mut Reactor<Tuple>) {
        let handler = unsafe { DynamicHandler::new(linked_lib_path("husky_handler.so").as_path()) };
        {
            let handler2 = unsafe { DynamicHandler::new(linked_lib_path("husky_handler.so").as_path()) };
            reactor.add_handler(Box::new(handler2));
        }

        reactor.add_handler(Box::new(handler));
    }

    #[test]
    fn dynamic_lib_loaded_twice_acts_correctly() {
        // let lib  = registry.create_handler(linked_lib_path("husky_handler.so").as_path());
        // registry.handlers.insert("husky_handler.so".to_string(), &lib);
        let mut reactor: Reactor<Tuple> = Reactor::new();
        let tuple = Tuple(vec![TupleNoun::Symbol("lexi".to_string()), TupleNoun::Symbol("is a".to_string()), TupleNoun::Symbol("husky".to_string())]);

        add_handler_to_reactor(&mut reactor);

        reactor.tick();
        reactor.insert(tuple);
        reactor.tick();

        let expected_tuple = Tuple(vec![TupleNoun::Symbol("lexi".to_string()), TupleNoun::Symbol("is".to_string()), TupleNoun::Symbol("cool".to_string())]);
        let cnt = reactor.ref_counts.get(&expected_tuple).unwrap();
        assert_eq!(cnt, &2);
    }
}