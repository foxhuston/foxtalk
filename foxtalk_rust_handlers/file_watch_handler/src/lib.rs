//
// mod commands_json_creator;
// mod cpp_handler_builder;
// mod reactor_adding_so_handler;
// mod recursive_inotify;
// mod reactor_debug_tuple_writer;
//
// use rust_tuple_reactor_serde::tuple::Tuple;
// use rust_tuple_reactor_serde::tuple_noun::TupleNoun;
// use rust_tuple_reactor_serde::*;
// use std::sync::LazyLock;
//
// #[allow(non_upper_case_globals)]
// #[no_mangle]
// pub static mut _foxtalk_ipc_buffer: [u8; 10*1024*1024] = [0; 10*1024*1024];
//
// static mut LATEST_SEEN_TUPLE: [Option<Tuple>; 1] = [None];
//
// struct DirectoryWatcher {
//     path_to_watch: String,
//     on_create: Box<dyn FnMut(String, String, String) -> ()>,
//     on_delete: Box<dyn FnMut(String, String, String) -> ()>,
// }
//
// impl DirectoryWatcher {
//     pub fn new(
//         path_to_watch: String,
//         on_create: Box<dyn FnMut(String, String, String) -> ()>,
//         on_delete: Box<dyn FnMut(String, String, String) -> ()>,
//     ) -> Self {
//         DirectoryWatcher {
//             path_to_watch,
//             on_create,
//             on_delete,
//         }
//     }
// }
//
// #[no_mangle]
// pub extern "C" fn init() {
//     let query_tuple = vec![Tuple(vec![
//         TupleNoun::Symbol("foxtalk reactor".to_string()),
//         TupleNoun::Symbol("is watching directory".to_string()),
//         TupleNoun::Prefix
//     ])];
//     unsafe { query_tuple.iter().write_to_buffer(_foxtalk_ipc_buffer.as_mut(), 0); }
//
// }
// #[no_mangle]
// pub extern "C" fn free_tuple() {
//
// }
// #[no_mangle]
// pub extern "C" fn handle() {
//     let (tuples, _) = unsafe { Vec::<Tuple>::read_from_buffer(_foxtalk_ipc_buffer.as_mut(), 0) };
//     for tuple in tuples {
//
//     }
// }
// #[no_mangle]
// pub extern "C" fn register_initial_tuples() {
// }
// #[no_mangle]
// pub extern "C" fn teardown() {
// }
// #[no_mangle]
// pub extern "C" fn poll() -> bool {
//     false
// }
