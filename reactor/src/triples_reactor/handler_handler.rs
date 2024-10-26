use crate::reactor::utils::non_aggregating_handler::NonAggregatingHandler;
use crate::triples_reactor::{Tuple, TupleNoun};
use std::collections::HashSet;
use std::path::PathBuf;
use std::rc::Rc;
use crate::triples_reactor::ffi::dynamic_handler::DynamicHandler;

struct HandlerHandler{}

impl NonAggregatingHandler<Tuple> for HandlerHandler {
    fn query(&mut self, o: &Tuple) -> bool {
        let Tuple(nouns) = o;
        if nouns.len() == 3 {
            match &nouns[..] {
                [TupleNoun::Symbol(s), TupleNoun::Symbol(p), TupleNoun::Symbol(o)] 
                    if p.eq("is a") && o.eq("handler") => {true}
                _ => {false}
            }
        } else  { false }
    }

    fn handle(&mut self, input: Tuple) -> HashSet<Tuple> {
        let first_noun = &input.0[0];
        match first_noun {
            TupleNoun::Symbol(s)=> {
                let handler = unsafe { DynamicHandler::new(PathBuf::from(s).as_path()) };
                let tuple = Tuple(vec![
                    TupleNoun::Symbol(s.clone()),
                    TupleNoun::Symbol("is a".to_string()),
                    TupleNoun::Symbol("handler".to_string()),
                    TupleNoun::Symbol("given by".to_string()),
                    TupleNoun::Handler(Rc::new(handler))    
                ]);
                HashSet::from([tuple])
            }
            _ => HashSet::new()
        }
    }
}
