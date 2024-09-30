use reactor::tuple::{Tuple, TupleNoun, test_helpers::{mk_tuple, mk_query}};
use reactor::when::When;

use reactor::ffi2::*;
use reactor::reactor::Reactor;

use std::path::PathBuf;


fn linked_lib_path(filename: &str) -> String {
    let mut path = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    path.push("tests/test_libs/out");
    path.push(filename);
    let new_path = path.clone();
    let path_str = new_path.to_str().unwrap();
    let owned_path = path_str.to_owned();
    owned_path
}

#[test]
fn ffi_loads_a_library() {
    unsafe { CWhen::new(linked_lib_path("test.so").as_str()) }.expect("opens the library");
}

#[test]
fn ffi_runs_handler() {
    let mut when = unsafe { CWhen::new(linked_lib_path("test.so").as_str()) }.unwrap();
    let query = when.get_query();

    println!("Got query: {query:?}");

    assert_eq!(query.subject, TupleNoun::Query());
    assert_eq!(query.predicate, TupleNoun::Symbol("is a".to_string()));
    assert_eq!(query.object, TupleNoun::Symbol("husky".to_string()));

    let results = when.handle(mk_tuple("lexi", "is a", "husky"));
    assert_eq!(results.len(), 1);

    let first = results.first().unwrap();
    assert_eq!(first.predicate, TupleNoun::Symbol("is a".to_string()));
}

#[test]
fn ffi_finds_tuples() {
    let mut reactor = Reactor::new();
    reactor.claim(mk_tuple("lexi", "is a", "husky"));

    let when = unsafe { CWhen::new(linked_lib_path("test_ids.so").as_str()) }.unwrap();
    reactor.add_handler(Box::new(when));

    reactor.tick();

    let results = reactor.db.query(mk_query(Some("lexi"), Some("is a"), None));

    println!("{:?}", results);

    assert_eq!(results.len(), 2);
}

#[test]
fn reactor_does_not_falsely_retrigger_handlers() {
    let mut reactor = Reactor::new();
    reactor.claim(mk_tuple("lexi", "is a", "husky"));

    let when = unsafe { CWhen::new(linked_lib_path("test_ids.so").as_str()) }.unwrap();
    reactor.add_handler(Box::new(when));

    reactor.tick();
    reactor.tick();
    reactor.tick();

    let results = reactor.db.query(mk_query(Some("lexi"), Some("is a"), None));

    println!("{:?}", results);

    assert_eq!(results.len(), 2);
}
