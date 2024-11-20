use std::collections::VecDeque;
use rust_handler_macro::*;
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;

struct RecursiveFileWatchingHandler {
    base_path: Option<String>,
    events: VecDeque<String>,
    
}

impl RecursiveFileWatchingHandler {
    
}

impl NeedsToImplement for RecursiveFileWatchingHandler {
    fn query(&self) -> Vec<Tuple> {
        vec![Tuple::triple_from_strs(&["foxtalk", "handlers", "exist", "at", "absolute", "path", "*"])]
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
        RecursiveFileWatchingHandler{ base_path: None, events: VecDeque::new() }
    }
}

foxtalk_handler!(RecursiveFileWatchingHandler);