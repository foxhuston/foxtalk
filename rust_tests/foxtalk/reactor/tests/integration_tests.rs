use reactor::query::Query;
use reactor::tuple::Tuple;
use reactor::when::When;

use reactor::ffi::*;
use reactor::reactor::Reactor;

use std::path::PathBuf;

fn linked_lib_path(filename: &str) -> &'static str {
    let mut path = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    path.push("tests/test_libs/out");
    path.push(filename);
    let path_str = path.to_str().unwrap().clone();

    // lol static str, Ser is refactoring the ffi
    Box::leak(path_str.to_string().into_boxed_str())

}

#[test]
fn ffi_loads_a_library() {
    let mut reg = LibraryRegistry::new();
    reg.cwhen_for(linked_lib_path("libtest.so.a")).unwrap();
}

#[test]
fn ffi_gets_query() {
    let mut reg = LibraryRegistry::new();
    let when = reg.cwhen_for(linked_lib_path("ac295b290ee73d16-test.o")).unwrap();

    let query = when.get_query();

    println!("Got query: {query:?}");

    assert_eq!(query.subject, None);
    assert_eq!(query.predicate, Some("Hi!".to_string()));
    assert_eq!(query.object, None);
}

#[test]
fn ffi_runs_handler() {
    let mut reg = LibraryRegistry::new();
    let mut when = reg.cwhen_for(linked_lib_path("libtest.so.a")).unwrap();

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

    let mut reg = LibraryRegistry::new();
    let when = reg.cwhen_for(linked_lib_path("libtest_ids.so.a")).unwrap();
    // reactor.add_handler(Box::new(when));
    println!("Got when: {:?}", when);
    reactor.tick();
    reactor.tick();
    reactor.tick();

    let results = reactor.db.query(Query::from_strs(Some("lexi"), Some("is a"), None));

println!("{:?}", results);

    assert_eq!(results.len(), 2);
}
