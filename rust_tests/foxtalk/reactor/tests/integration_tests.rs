use reactor::query::Query;
use reactor::tuple::Tuple;
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
fn ffi_gets_query() {
    let when = unsafe { CWhen::new(linked_lib_path("test.so").as_str()) }.unwrap();

    let query = when.get_query();

    println!("Got query: {query:?}");

    assert_eq!(query.subject, None);
    assert_eq!(query.predicate, Some("Hi!".to_string()));
    assert_eq!(query.object, None);
}

#[test]
fn ffi_runs_handler() {
    let mut when = unsafe { CWhen::new(linked_lib_path("test.so").as_str()) }.unwrap();
    let query = when.get_query();

    println!("Got query: {query:?}");

    assert_eq!(query.subject, None);
    assert_eq!(query.predicate, Some("Hi!".to_string()));
    assert_eq!(query.object, None);

    let results = when.handle(Tuple::new_strs("lexi", "is a", "husky"));
    assert_eq!(results.len(), 1);

    let first = results.first().unwrap();
    assert_eq!(first.predicate, "is a");

    results.into_iter().for_each(|x| unsafe { x.cleanup() });
}

#[test]
fn ffi_finds_tuples() {
    let mut reactor = Reactor::new();
    reactor.claim(Tuple::new_strs("lexi", "is a", "husky"));

    let when = unsafe { CWhen::new(linked_lib_path("test_ids.so").as_str()) }.unwrap();
    reactor.add_handler(Box::new(when));

    reactor.tick();
    reactor.tick();
    reactor.tick();

    let results = reactor.db.query(Query::from_strs(Some("lexi"), Some("is a"), None));

    println!("{:?}", results);

    assert_eq!(results.len(), 2);
}
