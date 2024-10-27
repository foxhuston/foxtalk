use std::collections::HashSet;
use crate::reactor::reactor_handler::Handler;
use crate::reactor::GeneratesHandler;
use crate::triples_reactor::ffi::dynamic_handler::DynamicHandler;
use std::path::Path;

pub mod serde;
pub mod ffi;

#[derive(Clone, Debug, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct Tuple(pub Vec<TupleNoun>);

impl Tuple {
    // #[cfg(test)]
    pub fn triple_from_sss(s: &str, p: &str, o: &str) -> Self {
        Tuple(vec![
            TupleNoun::Symbol(s.to_string()),
            TupleNoun::Symbol(p.to_string()),
            TupleNoun::Symbol(o.to_string())])
    }

    // #[cfg(test)]
    pub fn triple_from_ssu(s: &str, p: &str, o: u64) -> Self {
        Tuple(vec![TupleNoun::Symbol(s.to_string()),
                   TupleNoun::Symbol(p.to_string()),
                   TupleNoun::U64(o)])
    }

    fn is_handler_tuple(&self) -> Option<String> {
        let Tuple(nouns) = self;
        match &nouns[..] {
            [TupleNoun::Symbol(s), TupleNoun::Symbol(p), TupleNoun::Symbol(o)] if p == "is a" && o == "handler" => { Some(s.clone()) },
            _ => None
        }
    }

}

impl GeneratesHandler for Tuple {
    fn mk_handler_with_bootstrap_input(&self) -> Option<(Box<dyn Handler<Tuple>>, HashSet<Tuple>)> {
        if let Some(path) = self.is_handler_tuple() {
            let handler = unsafe { DynamicHandler::new(Path::new(&path)) };
            let bootstrapped_output = handler.get_bootstrap_output();
            Some((Box::new(handler), bootstrapped_output))
        } else {
            None
        }
    }
}

#[derive(Debug, PartialEq, Clone, Eq, Hash)]
pub enum TupleNoun {
    Query,          // 0
    Symbol(String), // 1
    CPtr(u64),      // 2
    U64(u64),       // 3
    I64(i64),       // 4
}

#[cfg(test)]
pub mod tests {
    use crate::reactor::Reactor;
    use crate::reactor::utils::test::linked_lib_path;
    use crate::triples_reactor::Tuple;

    #[test]
    pub fn it_should_add_and_remove_handlers() {
        let mut reactor = Reactor::new();

        let handler_tup = Tuple::triple_from_sss(linked_lib_path("husky_handler.so").to_str().unwrap(), "is a", "handler");
        let expected_tuple = Tuple::triple_from_sss("lexi", "is", "cool");
        reactor.insert(Tuple::triple_from_sss("lexi", "is a", "husky"));
        reactor.tick();

        assert_eq!(reactor.ref_counts.get(&expected_tuple), None);
        reactor.insert(handler_tup.clone());
        reactor.tick();
        reactor.tick();

        let cnt = reactor.ref_counts.get(&expected_tuple);
        assert_eq!(cnt, Some(&1));

        reactor.remove(handler_tup.clone());
        reactor.tick();
        reactor.tick();
        reactor.tick();
        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&expected_tuple), None);
        println!("Test end (Should have seen \"Dropping DynamicHandler\" before this...)");
    }
}
