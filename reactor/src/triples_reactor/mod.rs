use crate::reactor::{GeneratesProgram, ReactorData};
use crate::triples_reactor::ffi::dynamic_handler::DynamicallyLoadedProgram;
use std::path::Path;
use rust_tuple_reactor_serde::tuple::Tuple;
use rustc_hash::FxHashSet;
use crate::reactor::reactor_program::Program;

use log::*;

pub mod ffi;
pub mod triple_query_engine;

impl ReactorData for Tuple {}

impl GeneratesProgram<Tuple, Vec<Tuple>> for Tuple {
    fn mk_handler_with_bootstrap_input(&self) -> Option<(Box<dyn Program<Tuple, Vec<Tuple>>>, FxHashSet<Tuple>)> {
        if let Some(path) = self.is_handler_tuple() {
            match unsafe { DynamicallyLoadedProgram::new(Path::new(&path)) } {
                Ok(handler) => {
                    let bootstrapped_output = handler.get_bootstrap_output();
                    Some((Box::new(handler), bootstrapped_output))
                }
            
                Err(e) => {
                    error!("Error loading handler: {:?}", e);
                    None
                }
            }
        } else {
            None
        }
    }
}



#[cfg(test)]
pub mod tests {
    use crate::reactor::Reactor;
    use crate::reactor::utils::test::linked_lib_path;
    use crate::triples_reactor::triple_query_engine::TripleQueryEngine;
    use crate::triples_reactor::Tuple;
    
    use log::{error, warn, info, debug, trace};

    #[test]
    pub fn it_should_add_and_remove_handlers() {
        let query_engine = TripleQueryEngine::new();
        let mut reactor = Reactor::new(query_engine);

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
        trace!("Test end (Should have seen \"Dropping DynamicHandler\" before this...)");
    }
}
