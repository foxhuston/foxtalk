// mod fox_experiments;
pub mod reactor_handler;
pub mod utils;

use crate::reactor::reactor_handler::Handler;
use reactor_handler::ReactorHandler;
use std::collections::{HashMap, HashSet};
use std::fmt::Debug;
use std::hash::Hash;
/*
 * Reactor has handlers.
 * These handlers live _at most_ as long as Reactor.
 *
 * TupleReactor (specifically) watches its own database (somehow) for tuples that describe handlers
 */

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
#[repr(transparent)]
pub struct ReactorHandlerId(u64);

pub trait GeneratesHandler where Self: Eq + Hash + Sized {
    fn mkHandler(&self) -> Option<Box<dyn Handler<Self>>> { None }
}

impl GeneratesHandler for u64 { }


pub struct Reactor<O: Eq + Hash + GeneratesHandler + Clone + Debug> {
    pub handlers: HashMap<ReactorHandlerId, ReactorHandler<O>>,
    pub ref_counts: HashMap<O, u64>,
    
    generated_handlers: HashMap<O, ReactorHandlerId>,
    current_handler_id: u64
}

impl<O: Eq + Hash + GeneratesHandler + Clone + Debug> Reactor<O> {
    pub fn new( ) -> Self {
        Reactor {
            handlers: HashMap::new(),
            ref_counts: HashMap::new(),
            generated_handlers: HashMap::new(),
            current_handler_id: 0
        }
    }

    pub fn add_handler_with_bootstrap_output(&mut self, handler: Box<dyn Handler<O>>, O: HashSet<O>) -> ReactorHandlerId {
        let mut handler = ReactorHandler::new_with_bootstrap_output(handler, O);

        for output in &handler.O {
            self.insert(output.clone());
        }

        let all_outputs: HashSet<&O> = self.ref_counts.keys().collect();
        for output in all_outputs {
            if handler.query(output) {
                handler.I.insert(output.clone());
            }
        }
        handler.dirty = true;
        
        let id = ReactorHandlerId(self.current_handler_id);
        self.current_handler_id += 1;
        self.handlers.insert(id.clone(), handler);
        
        id
    }

    pub fn add_handler(&mut self, handler: Box<dyn Handler<O>>) -> ReactorHandlerId {
        self.add_handler_with_bootstrap_output(handler, HashSet::new())
    }
    
    pub fn remove_handler(&mut self, handler_id: ReactorHandlerId) {
        println!("Removing handler {:?}", handler_id);
        
        
        if let Some(handler) = self.handlers.get(&handler_id) {
            let output = handler.O.clone();
            for o in output {
                self.remove_with_hid(o, Some(handler_id.clone()));
            }
        }
    }

    pub fn insert(&mut self, input: O) {
        println!("adding o: {:?}", input);
        if self.ref_counts.contains_key(&input) {
            let count = self.ref_counts.get_mut(&input).unwrap();
            *count += 1;
        } else {
            if let Some(h) = input.mkHandler() {
                let hid = self.add_handler(h);
                self.generated_handlers.insert(input.clone(), hid);
            }
            
            for (_, handler) in self.handlers.iter_mut() {
                if handler.query(&input) && !handler.I.contains(&input) {
                    handler.I.insert(input.clone());
                    handler.dirty = true;
                }
            }
            self.ref_counts.insert(input, 1);
        }
    }


    pub fn remove(&mut self, input: O) {
        self.remove_with_hid(input, None);
    }
    fn remove_with_hid(&mut self, input: O, maybe_hid: Option<ReactorHandlerId>) {
        println!("removing o: {:?}", input);
        if self.ref_counts.contains_key(&input) {
            let count = self.ref_counts.get_mut(&input).unwrap();
            *count -= 1;
            if *count == 0 {
                if let Some(hid) = maybe_hid {
                    self.handlers.get_mut(&hid).unwrap().free_o(&input);
                }
                
                if let Some(hid) = self.generated_handlers.remove(&input) {
                    self.remove_handler(hid);
                }
                
                self.ref_counts.remove(&input);
                
                for (_, handler) in self.handlers.iter_mut() {
                    if handler.I.contains(&input) {
                        handler.I.remove(&input);
                        handler.dirty = true;
                    }
                }
            }
        }
    }

    pub fn tick(&mut self) {
        println!("tick tick tick");
        let mut need_to_insert_total = Vec::new();
        let mut need_to_remove_total = Vec::new();
        for (hid, handler) in self.handlers.iter_mut() {
            if handler.dirty {
                // let qa = &mut handler.qa;
                // let input = &handler.I;

                let new_output = handler.handle();

                let need_to_insert: HashSet<O> = new_output.difference(&handler.O).cloned().collect();
                let need_to_remove: HashSet<O> = handler.O.difference(&new_output).cloned().collect();

                for i in need_to_insert.into_iter() {
                    need_to_insert_total.push(i);
                }

                for i in need_to_remove.into_iter() {
                    need_to_remove_total.push((i, hid.clone()));
                }
                handler.O = new_output;
                handler.dirty = false;

            }

        }

        for i in need_to_insert_total.into_iter() {
            self.insert(i);
        }
            
        for (i, hid) in need_to_remove_total.into_iter() {
            self.remove_with_hid(i, Some(hid));
        }
    }


}

impl<O: Eq + Hash + GeneratesHandler + Clone + Debug>  Drop for Reactor<O> {
    fn drop(&mut self) {
        let hids: Vec<ReactorHandlerId> = self.handlers.keys().cloned().collect();
        
        for hid in hids {
            self.remove_handler(hid);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::reactor::reactor_handler::Handler;
    use std::collections::HashSet;

    #[test]
    pub fn paper_agg_example() {
        struct PaperHandler;
        impl Handler<u64> for PaperHandler {
            fn query(&mut self, o: &u64) -> bool {
                *o > 5 && *o < 20
            }
            fn handle(&mut self, o: &HashSet<u64>) -> HashSet<u64> {
                let mut sum = 0;
                for &i in o {
                    sum += i;
                }
                let mut out = HashSet::new();
                out.insert(sum);
                out
            }
        }

        let mut reactor = Reactor::new();
        let handler = Box::new(PaperHandler);
        let hid = reactor.add_handler(handler);

        reactor.tick();
        reactor.insert(3);
        reactor.insert(7);
        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&3).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&7).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&7).unwrap(), &2);

        assert_eq!(reactor.handlers.get(&hid).unwrap().dirty, false);
        reactor.insert(10);
        assert_eq!(reactor.handlers.get(&hid).unwrap().dirty, true);
        assert_eq!(reactor.handlers.get(&hid).unwrap().I.contains(&10), true);
        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);
    }
}