use std::fs;
use anyhow::{format_err, Result};

use crate::reactor::reactor_program::Program;
use crate::triples_reactor::Tuple;
use libloading::os::unix::{Library, Symbol, RTLD_LOCAL, RTLD_NOW};
use rust_tuple_reactor_serde::{FoxTalkDeserializable, FoxTalkOwnedSerializable, FoxTalkSerializable};
use rustc_hash::FxHashSet;
use std::path::Path;
use log::debug;
use uuid::Uuid;

#[derive(Debug)]
pub struct DynamicallyLoadedProgram {
    _lib: Library,
    _init: Symbol<extern "C" fn(*mut u8) -> ()>,
    free_tuple: Symbol<extern "C" fn(*mut u8) -> ()>,
    _register_initial_tuples: Symbol<extern "C" fn(*mut u8) -> ()>,
    handle: Symbol<extern "C" fn(*mut u8) -> ()>,
    teardown: Symbol<extern "C" fn() -> ()>,
    buffer: Vec<u8>,
    poll: Symbol<extern "C" fn() -> bool>,
    query: Vec<Tuple>,
    _original_path: String,
    _copied_path: String,
}

unsafe impl Send for DynamicallyLoadedProgram {}

static BUFFER_SIZE: usize = 10 * 1024 * 1024;

impl DynamicallyLoadedProgram {
    pub fn get_bootstrap_output(&self) -> FxHashSet<Tuple> {
        let (res, _) = Vec::<Tuple>::read_from_buffer(&self.buffer, 0);
        FxHashSet::from_iter(res)
    }
    pub unsafe fn new(path: &Path) -> Result<Self> {

        // Copy the so to a temp location and load it from there.
        let tmp_path = std::env::var("SO_TMP_PATH")?;
        let id = Uuid::new_v4();
        let original_file_name = path.file_name().unwrap().to_str().unwrap_or("unknown");
        
        let full_path = format!("{}/{}-{}", tmp_path, id.as_u128()%1000000000, original_file_name);
        
        fs::copy(path, full_path.clone())?;

        // Magic 0x08 number is DEEPBIND for dlopen.
        let lib = Library::open(Some(full_path.clone()), RTLD_NOW | RTLD_LOCAL | 0x08)?;
        debug!("Opened library {:?} ({:?}) [tmp path: {:?}", path, lib, full_path.clone());

        let init: Symbol<extern "C" fn(*mut u8) -> ()> = lib.get(b"foxtalk_init")?;
        let free_tuple: Symbol<extern "C" fn(*mut u8) -> ()> = lib.get(b"foxtalk_free_tuple")?;
        let register_initial_tuples: Symbol<extern "C" fn(*mut u8) -> ()> = lib.get(b"foxtalk_register_initial_tuples")?;
        let handle: Symbol<extern "C" fn(*mut u8) -> ()> = lib.get(b"foxtalk_handle")?;
        let teardown: Symbol<extern "C" fn() -> ()> = lib.get(b"foxtalk_teardown")?;
        let poll: Symbol<extern "C" fn() -> bool> = lib.get(b"foxtalk_poll")?;
        let mut buffer = vec![0; BUFFER_SIZE];
        init(buffer.as_mut_ptr());
        let (query, _) = Vec::<Tuple>::read_from_buffer(&buffer, 0);

        if query.is_empty() {
            return Err(format_err!("Query not loaded into the buffer after calling init on {:?}. Not loading program.", path));
        }

        register_initial_tuples(buffer.as_mut_ptr());

        Ok(DynamicallyLoadedProgram {
            _lib: lib,
            _init: init,
            free_tuple,
            _register_initial_tuples: register_initial_tuples,
            handle,
            teardown,
            buffer,
            poll,
            query,
            _original_path: path.to_str().unwrap().to_string(),
            _copied_path: full_path.clone(),
        })
    }
}

impl Drop for DynamicallyLoadedProgram {
    fn drop(&mut self) {
        debug!("Dropping DynamicallyLoadedProgram {:?}", self._lib);
        (self.teardown)();
    }
}

impl Program<Tuple, Vec<Tuple>> for DynamicallyLoadedProgram {
    fn query(&mut self) -> Vec<Tuple> {
        self.query.clone()
    }

    fn handle(&mut self, input: &FxHashSet<Tuple>) -> FxHashSet<Tuple> {
        input.into_iter().write_to_buffer(&mut self.buffer, 0);
        (self.handle)(self.buffer.as_mut_ptr());
        let (res, _) = Vec::<Tuple>::read_from_buffer(&mut self.buffer, 0);
        FxHashSet::from_iter(res)
    }
    fn poll(&mut self) -> bool {
        (self.poll)()
    }
    fn free_o(&mut self, o: &Tuple) -> () {
        o.write_to_buffer(&mut self.buffer, 0);
        (&self.free_tuple)(self.buffer.as_mut_ptr())
    }
}