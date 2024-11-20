use rust_handler_macro::*;
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;

struct HuskyHandler;

impl NeedsToImplement for HuskyHandler {
    fn query(&self) -> Vec<Tuple> {
        vec![Tuple::triple_from_strs(&["*", "is a", "husky"])]
    }

    fn handle(&self, tuples: Vec<Tuple>) -> Option<Vec<Tuple>> {
        let x = tuples.iter().map(|tuple| {
            let Tuple(nouns) = tuple;
            match &nouns[..] {
                [TupleNoun::Symbol(s), _, _] => {
                    Some(Tuple::triple_from_strs(&[s, "is", "cool"]))
                }
                _ => None
            }
        }).filter_map(|x| x).collect();
        Some(x)
        
    }

    fn new() -> Self {
        HuskyHandler{}
    }
}

foxtalk_handler!(HuskyHandler);