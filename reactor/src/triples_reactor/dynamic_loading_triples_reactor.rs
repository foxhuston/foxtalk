use std::collections::HashSet;
use std::path::Path;
use crate::reactor::Reactor;
use crate::triples_reactor::{Tuple, TupleNoun};
use crate::triples_reactor::ffi::dynamic_handler::DynamicHandler;

struct DynamicLoadingTriplesReactor {
    reactor: Reactor<Tuple>,
    current_handlers: HashSet<String>
}

impl DynamicLoadingTriplesReactor {
    pub fn new() -> Self {
        DynamicLoadingTriplesReactor {
            reactor: Reactor::new(),
            current_handlers: HashSet::new()
        }
    }
    
    pub fn insert(&mut self, t: Tuple) {
        self.reactor.insert(t)
    }
    
    pub fn remove(&mut self, t: Tuple) {
        self.reactor.remove(t)
    }
    
    fn is_handler_tuple(Tuple(nouns): &Tuple) -> Option<String> {
        match &nouns[..] {
            [TupleNoun::Symbol(s), TupleNoun::Symbol(p), TupleNoun::Symbol(o)] if p == "is a" && o == "handler" => { Some(s.clone()) },
            _ => None
        }
    }
    
    pub fn tick(&mut self) {
        self.reactor.tick();
        let mut handlers_to_insert = Vec::new();
        let mut seen = HashSet::new();
        
        for tuple in self.reactor.ref_counts.keys()
        {
            if let Some(path) = Self::is_handler_tuple(&tuple) {
                seen.insert(path.clone());
                
                if !self.current_handlers.contains(&path) {
                    self.current_handlers.insert(path.clone());
                    handlers_to_insert.push(Box::new(unsafe { DynamicHandler::new(Path::new(&path)) }));
                }
            }
        }  
        
        let to_remove: HashSet<&String> = self.current_handlers.difference(&seen).collect();
        for path in to_remove {
            // self.current_handlers.remove(path);
        }
    }
}