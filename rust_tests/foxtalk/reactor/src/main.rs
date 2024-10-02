use std::path::PathBuf;
use tuple_db::tuple::test_helpers::{mk_query, mk_tuple};
use reactor::ffi2::CWhen;
use reactor::reactor::Reactor;

fn linked_lib_path(filename: &str) -> String {
    let mut path = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
    path.push("tests/test_libs/out");
    path.push(filename);
    let new_path = path.clone();
    let path_str = new_path.to_str().unwrap();
    let owned_path = path_str.to_owned();
    owned_path
}

fn main() {
    let mut reactor = Reactor::new();
    reactor.claim(mk_tuple("lexi", "is a", "husky"));

    let when = unsafe { CWhen::new(linked_lib_path("test_ids.so").as_str()) }.unwrap();
    reactor.add_handler(Box::new(when));

    reactor.tick();
    reactor.tick();
    reactor.tick();

    let results = reactor.db.query(mk_query(Some("lexi"), Some("is a"), None));

    println!("{:?}", results);
}