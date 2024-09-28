use libloading::Error::CreateCStringWithTrailing;
use reactor::db::Db;
use reactor::when::When;
use reactor::query::Query;
use reactor::tuple::Tuple;
use reactor::tuple::TupleNoun::Str;

use reactor::ffi::*;
use reactor::reactor::Reactor;

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

    let mut results = when.handle(Tuple::new_strs("lexi", "is a", "husky"));
    assert_eq!(results.len(), 1);

    let first = results.first().unwrap();
    assert_eq!(first.predicate, "is a");

    results.into_iter().for_each(|x| unsafe { x.cleanup() });
}

#[test]
fn ffi_finds_tuples() {
    let mut reactor = Reactor::new();
    reactor.claim(Tuple::new_strs("lexi", "is a", "husky"));

    let mut reg = LibraryRegistry::new();
    let when = reg.cwhen_for("/home/fox/dev/foxtalk-test/rust_tests/reactor/target/debug/libtest_ids.so").unwrap();
    reactor.add_handler(Box::new(when));

    reactor.tick();
    reactor.tick();
    reactor.tick();

    let results = reactor.db.query(Query::from_strs(Some("lexi"), Some("is a"), None));

println!("{:?}", results);

    assert_eq!(results.len(), 2);
}
