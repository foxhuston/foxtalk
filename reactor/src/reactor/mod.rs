// mod fox_experiments;
pub mod reactor_handler;
pub mod utils;

use std::collections::{HashMap, HashSet};
use std::fmt::Debug;
use std::hash::Hash;
use reactor_handler::ReactorHandler;
use crate::reactor::reactor_handler::Handler;
/*
 * Reactor has handlers.
 * These handlers live _at most_ as long as Reactor.
 *
 * TupleReactor (specifically) watches its own database (somehow) for tuples that describe handlers
 */


pub struct Reactor<O: Eq + Hash> {
    pub handlers: Vec<ReactorHandler<O>>,
    pub ref_counts: HashMap<O, u64>
}

impl<O: Eq + Hash + Clone + Debug> Reactor<O> {
    pub fn new() -> Self {
        Reactor {
            handlers: Vec::new(),
            ref_counts: HashMap::new()
        }
    }

    pub fn add_handler_with_bootstrap_output(&mut self, handler: Box<dyn Handler<O>>, O: HashSet<O>) {
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
        self.handlers.push(handler);
    }

    pub fn add_handler(&mut self, handler: Box<dyn Handler<O>>) {
        self.add_handler_with_bootstrap_output(handler, HashSet::new())
    }

    pub fn insert(&mut self, input: O) {
        println!("adding o: {:?}", input);
        if self.ref_counts.contains_key(&input) {
            let count = self.ref_counts.get_mut(&input).unwrap();
            *count += 1;
        } else {

            for handler in self.handlers.iter_mut() {

                if handler.query(&input) && !handler.I.contains(&input) {
                    handler.I.insert(input.clone());
                    handler.dirty = true;
                }
            }
            self.ref_counts.insert(input, 1);
        }
    }

    pub fn remove(&mut self, input: O) {
        println!("removing o: {:?}", input);
        if self.ref_counts.contains_key(&input) {
            let count = self.ref_counts.get_mut(&input).unwrap();
            *count -= 1;
            if *count == 0 {
                self.ref_counts.remove(&input);
                for handler in self.handlers.iter_mut() {
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
        for handler in self.handlers.iter_mut() {
            if handler.dirty {
                // let qa = &mut handler.qa;
                // let input = &handler.I;

                // TODO: THIS HAS A BUG
                // Example: Agg handler counting number of numbers and adding that count in output set
                // then all the inputs it cares about get removed
                // if !input.is_empty() {
                let new_output = handler.handle();

                let need_to_insert: HashSet<O> = new_output.difference(&handler.O).cloned().collect();
                let need_to_remove: HashSet<O> = handler.O.difference(&new_output).cloned().collect();

                for i in need_to_insert.into_iter() {
                    need_to_insert_total.push(i);
                }

                for i in need_to_remove.into_iter() {
                    need_to_remove_total.push(i);
                }
                handler.O = new_output;
                // }
                handler.dirty = false;

            }

        }

        for i in need_to_insert_total.into_iter() {
            self.insert(i);
        }
        for i in need_to_remove_total.into_iter() {
            self.remove(i);
        }
    }


}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashSet;
    use crate::reactor::reactor_handler::Handler;

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
        reactor.add_handler(handler);

        reactor.tick();
        reactor.insert(3);
        reactor.insert(7);
        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&3).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&7).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&7).unwrap(), &2);

        assert_eq!(reactor.handlers.get(0).unwrap().dirty, false);
        reactor.insert(10);
        assert_eq!(reactor.handlers.get(0).unwrap().dirty, true);
        assert_eq!(reactor.handlers.get(0).unwrap().I.contains(&10), true);
        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);
    }
}