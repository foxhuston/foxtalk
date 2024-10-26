// use std::collections::{HashMap, HashSet};
// use std::path::Path;
// use crate::reactor::{Reactor, ReactorHandlerId};
// use crate::triples_reactor::{Tuple, TupleNoun};
// use crate::triples_reactor::ffi::dynamic_handler::DynamicHandler;
//
// struct DynamicLoadingTriplesReactor {
//     reactor: Reactor<Tuple>,
// }
//
// impl DynamicLoadingTriplesReactor {
//     pub fn new() -> Self {
//         DynamicLoadingTriplesReactor {
//             reactor: Reactor::new(),
//         }
//     }
//
//     pub fn insert(&mut self, t: Tuple) {
//         self.reactor.insert(t)
//     }
//
//     pub fn remove(&mut self, t: Tuple) {
//         self.reactor.remove(t)
//     }
//
//     fn is_handler_tuple(Tuple(nouns): &Tuple) -> Option<String> {
//         match &nouns[..] {
//             [TupleNoun::Symbol(s), TupleNoun::Symbol(p), TupleNoun::Symbol(o)] if p == "is a" && o == "handler" => { Some(s.clone()) },
//             _ => None
//         }
//     }
//
//     pub fn tick(&mut self) {
//         self.reactor.tick();
//         let mut handlers_to_insert = Vec::new();
//         let mut seen = HashSet::new();
//         // TODO: I really don't like having to iterate over every object in the DB every tick... this is
//         //       *EXACTLY* what the entire reactor algorithm is for...
//         for tuple in self.reactor.ref_counts.keys()
//         {
//             if let Some(path) = Self::is_handler_tuple(&tuple) {
//                 seen.insert(tuple.clone());
//
//                 if !self.current_handlers.contains_key(&tuple) {
//                     handlers_to_insert.push((tuple.clone(), Box::new(unsafe { DynamicHandler::new(Path::new(&path)) })));
//                 }
//             }
//         }
//
//         for (path, handler) in handlers_to_insert {
//             let hid = self.reactor.add_handler(handler);
//             self.current_handlers.insert(path, hid);
//         }
//
//         let current_keyset: HashSet<Tuple> = self.current_handlers.keys().cloned().collect();
//         let to_remove = current_keyset.difference(&seen);
//         for path in to_remove {
//             match self.current_handlers.remove(path) {
//                 None => { println!("WARNING: DynamicLoadingTriplesReactor tried to remove a handler that was not present in its dictionary"); }
//                 Some(hid) => {
//                     self.reactor.remove_handler(hid);
//                 }
//             }
//         }
//     }
// }
//
// #[cfg(test)]
// pub mod tests {
//     use crate::triples_reactor::dynamic_loading_triples_reactor::DynamicLoadingTriplesReactor;
//     use crate::triples_reactor::ffi::dynamic_handler::test::linked_lib_path;
//     use crate::triples_reactor::{Tuple, TupleNoun};
//
//     #[test]
//     pub fn it_should_add_and_remove_handlers() {
//         let mut reactor = DynamicLoadingTriplesReactor::new();
//
//         let handler_tup = Tuple::triple_from_strs(linked_lib_path("husky_handler.so").to_str().unwrap(), "is a", "handler");
//         let expected_tuple = Tuple::triple_from_strs("lexi", "is", "cool");
//
//         reactor.insert(Tuple::triple_from_strs("lexi", "is a", "husky"));
//         reactor.tick();
//
//         assert_eq!(reactor.reactor.ref_counts.get(&expected_tuple), None);
//
//         reactor.insert(handler_tup.clone());
//         reactor.tick();
//         reactor.tick();
//
//         let cnt = reactor.reactor.ref_counts.get(&expected_tuple);
//         assert_eq!(cnt, Some(&1));
//
//         reactor.remove(handler_tup.clone());
//         reactor.tick();
//
//         assert_eq!(reactor.reactor.ref_counts.get(&expected_tuple), None);
//         println!("Test end (Should have seen \"Dropping DynamicHandler\" before this...)");
//     }
// }