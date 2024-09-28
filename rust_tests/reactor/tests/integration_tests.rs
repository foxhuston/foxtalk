use reactor::db::Db;
use reactor::when::When;
use reactor::query::Query;
use reactor::tuple::Tuple;
use reactor::tuple::TupleNoun::Str;

use reactor::ffi::*;

#[test]
fn ffi_loads_a_library() {
    let mut reg = LibraryRegistry::new();
    let when = reg.cwhen_for("libtest.so").unwrap();
}

#[test]
fn ffi_gets_query() {
    let mut reg = LibraryRegistry::new();
    // let when = reg.cwhen_for("/home/fox/dev/foxtalk_test/rust_tests/reactor/target/debug/libtest.so").unwrap();
    let when = reg.cwhen_for("/home/fox/dev/foxtalk-test/rust_tests/reactor/target/debug/libtest.so").unwrap();

    let query = when.get_query();

    println!("Got query: {query:?}");

    assert_eq!(query.subject, None);
    assert_eq!(query.predicate, Some("Hi!".to_string()));
    assert_eq!(query.object, None);
}

#[test]
fn ffi_runs_handler() {
    let mut reg = LibraryRegistry::new();
    // let when = reg.cwhen_for("/home/fox/dev/foxtalk_test/rust_tests/reactor/target/debug/libtest.so").unwrap();
    let mut when = reg.cwhen_for("/home/fox/dev/foxtalk-test/rust_tests/reactor/target/debug/libtest.so").unwrap();

    let query = when.get_query();

    println!("Got query: {query:?}");

    assert_eq!(query.subject, None);
    assert_eq!(query.predicate, Some("Hi!".to_string()));
    assert_eq!(query.object, None);

    let results = when.handle(Tuple::new_strs("lexi", "is a", "husky"));
    assert_eq!(results.len(), 1);

    let first = results.first().unwrap();
    assert_eq!(first.predicate, "is highlighted");
}
