use rust_handler_macro::*;
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;
use std::sync::Arc;
use parking_lot::Mutex;

struct HuskyHandler;

impl NeedsToImplement for HuskyHandler {
    fn query(&mut self) -> Vec<Tuple> {
        vec![Tuple::triple_from_strs(&["*", "is a", "husky"])]
    }

    fn handle(&mut self, tuples: Vec<Tuple>) -> Vec<Tuple> {
        let x = tuples.iter().map(|tuple| {
            let Tuple(nouns) = tuple;
            match &nouns[..] {
                [TupleNoun::Symbol(s), _, _] => {
                    Some(Tuple::triple_from_strs(&[s, "is", "cool"]))
                }
                _ => None
            }
        }).filter_map(|x| x).collect();
        x
    }

    fn new() -> Self {
        HuskyHandler{}
    }
}

foxtalk_handler!(HuskyHandler);