use reactor::triples_reactor::serde::FoxTalkSerializable;
use reactor::utils::ReactorHandle;
use std::path::{Path, PathBuf};

fn linked_lib_path(filename: &str) -> PathBuf {
    let mut path = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    path.push("tests/test_libs/out");
    path.push(filename);
    let new_path = path.clone();
    let path_str = new_path.to_str().unwrap();
    let owned_path = path_str;
    PathBuf::from(owned_path)
}
//
#[cfg(test)]
mod tests {
    use reactor::reactor::Reactor;
    use reactor::triples_reactor::ffi::HandlerRegistry;
    use reactor::triples_reactor::{Tuple, TupleNoun};
    use reactor::utils::ReactorHandler;
    use crate::linked_lib_path;

    #[test]
    fn ffi_loads_a_library() {
        let mut registry: HandlerRegistry = HandlerRegistry::new();

        let lib  = registry.create_handler(linked_lib_path("husky_handler.so").as_path());
        // registry.handlers.insert("husky_handler.so".to_string(), &lib);


        let handler = ReactorHandler::new(Box::new(lib));

        let tuple = Tuple(vec![TupleNoun::Symbol("lexi".to_string()), TupleNoun::Symbol("is a".to_string()), TupleNoun::Symbol("husky".to_string())]);
        let mut reactor: Reactor<Tuple> = Reactor::new();
        reactor.add_handler(handler);
        reactor.tick();
        reactor.insert(tuple);
        reactor.tick();

        let expected_tuple = Tuple(vec![TupleNoun::Symbol("lexi".to_string()), TupleNoun::Symbol("is".to_string()), TupleNoun::Symbol("cool".to_string())]);
        let cnt = reactor.ref_counts.get(&expected_tuple).unwrap();
        assert_eq!(cnt, &1);
    }
}

//
// #[test]
// fn ffi_runs_handler() {
//     let mut when = unsafe { CWhen::new(linked_lib_path("test.so").as_str()) }.unwrap();
//     let query = when.get_query();
//
//     println!("Got query: {query:?}");
//
//     assert_eq!(query.subject, TupleNoun::Query());
//     assert_eq!(query.predicate, TupleNoun::from_str("is a"));
//     assert_eq!(query.object, TupleNoun::from_str("husky"));
//
//     let results = when.handle(mk_tuple("lexi", "is a", "husky"));
//     assert_eq!(results.len(), 1);
//
//     let first = results.first().unwrap();
//     assert_eq!(first.predicate, TupleNoun::from_str("is a"));
// }
//
// #[test]
// fn ffi_finds_tuples() {
//     let mut reactor = Reactor::new();
//     reactor.claim(mk_tuple("lexi", "is a", "husky"));
//
//     let when = unsafe { CWhen::new(linked_lib_path("test_ids.so").as_str()) }.unwrap();
//     reactor.add_handler(Box::new(when));
//
//     reactor.tick();
//
//     let results = reactor.db.query(mk_query(Some("lexi"), Some("is a"), None));
//
//     println!("{:?}", results);
//
//     assert_eq!(results.len(), 2);
// }
//
// #[test]
// fn reactor_does_not_falsely_retrigger_handlers() {
//     let mut reactor = Reactor::new();
//     reactor.claim(mk_tuple("lexi", "is a", "husky"));
//
//     let when = unsafe { CWhen::new(linked_lib_path("test_ids.so").as_str()) }.unwrap();
//     reactor.add_handler(Box::new(when));
//
//     reactor.tick();
//     reactor.tick();
//     reactor.tick();
//
//     let results = reactor.db.query(mk_query(Some("lexi"), Some("is a"), None));
//
//     println!("{:?}", results);
//
//     assert_eq!(results.len(), 2);
// }
