mod reactor_handler;
pub use reactor_handler::*;

use std::collections::{HashMap, HashSet};
use std::hash::Hash;



// Deciding explicitly that we _copy_ o's into their
// respective input sets.
pub struct Reactor<O: Eq + Hash>
{
    pub handlers: Vec<ReactorHandler<O>>,
    pub ref_counts: HashMap<O, usize>,
}

impl<O: Eq + Hash + Clone> Reactor<O> {
    pub fn new() -> Reactor<O> {
        Reactor {
            handlers: Vec::new(),
            ref_counts: HashMap::new(),
        }
    }

    pub fn add_handler(&mut self, mut handler: ReactorHandler<O>) {
        for output in &handler.O {
            self.insert(output.clone());
        }

        let all_outputs = self.ref_counts.keys().collect::<HashSet<&O>>();
        for output in all_outputs {
            if handler.query(output) {
                handler.I.insert(output.clone());
            }
        }

        handler.dirty = true;
        self.handlers.push(handler);
    }

    pub fn insert(&mut self, input: O) {
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
            self.ref_counts.insert(input.clone(), 1);
        }
    }

    pub fn remove(&mut self, input: O) {
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
        println!("tick!");
        let mut need_to_insert_total = HashSet::new();
        let mut need_to_remove_total = HashSet::new();
        for handler in self.handlers.iter_mut() {
            if handler.dirty {
                let input = handler.I.iter().collect::<HashSet<&O>>();
                // TODO: This causes a SIGSERV error somewhere if this if isn't here
                // Figure out why
                if !input.is_empty() {
                    let new_output = handler.handle();

                    let need_to_insert = new_output.difference(&handler.O).cloned().collect::<HashSet<O>>();
                    for i in need_to_insert.iter() {
                        need_to_insert_total.insert(i.clone());
                    }
                    let need_to_remove = handler.O.difference(&new_output).cloned().collect::<HashSet<O>>();
                    for i in need_to_remove.iter() {
                        need_to_remove_total.insert(i.clone());
                    }
                    handler.O = new_output;
                }
                handler.dirty = false;

            }
        }

        for i in need_to_insert_total.iter() {
            self.insert(i.clone());
        }
        for i in need_to_remove_total.iter() {
            self.remove(i.clone());
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashSet;
    use std::fmt::Debug;


    #[test]
    pub fn paper_agg_example() {

        fn paper_query(other: &u64) -> bool {
            *other > 5 && *other < 20
        }
        fn agg(o: HashSet<&u64>) -> HashSet<u64> {
            let mut sum = 0;
            for i in o {
                sum += i;
            }
            let mut out = HashSet::new();
            out.insert(sum);
            out
        }

        let a = &mut agg;
        let q = &mut paper_query;

        struct PaperHandler;
        impl Handler<u64> for PaperHandler {
            fn query(&self, o: &u64) -> bool {
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
        let handler = ReactorHandler::new(Box::new(PaperHandler));
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

    #[test]
    pub fn paper_non_agg_example() {
        struct PaperHandler;
        impl Handler<u64> for PaperHandler {
            fn query(&self, o: &u64) -> bool {
                *o == 1 || *o == 2 || *o == 3
            }
            fn handle(&mut self, o: &HashSet<u64>) -> HashSet<u64> {
                let mut results = HashSet::new();
                for &i in o {
                    match i {
                        1 => {
                            results.insert(2);
                            results.insert(3);
                        }
                        2 => {
                            results.insert(5);
                        }
                        _ => {}
                    }
                }
                results
            }
        }

        let mut reactor = Reactor::new();
        let handler = ReactorHandler::new(Box::new(PaperHandler));
        reactor.add_handler(handler);

        reactor.tick();
        // So if we call: 𝗂𝗇𝗌𝖾𝗋𝗍(𝔇0, 𝑜1), then...
        reactor.insert(1);
        assert_eq!(reactor.handlers.get(0).unwrap().dirty, true);
        assert_eq!(reactor.handlers.get(0).unwrap().I, HashSet::from_iter(vec![1]));
        assert_eq!(reactor.ref_counts.get(&1).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&1).unwrap(), &1);

        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&1).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&1).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&2).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);

        assert_eq!(reactor.handlers.get(0).unwrap().dirty, true);
        assert_eq!(reactor.handlers.get(0).unwrap().O, HashSet::from_iter(vec![2, 3]));
        // assert_eq!(reactor.handlers.get(0).unwrap().S.get(&1).unwrap(), &HashSet::from_iter(vec![2, 3]));
        assert_eq!(reactor.handlers.get(0).unwrap().I, HashSet::from_iter(vec![1, 2, 3]));

        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&1).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&1).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&2).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&5).unwrap(), &1);

        assert_eq!(reactor.handlers.get(0).unwrap().dirty, false);
        assert_eq!(reactor.handlers.get(0).unwrap().O, HashSet::from_iter(vec![2, 3, 5]));
        // assert_eq!(reactor.handlers.get(0).unwrap().S.get(&1).unwrap(), &HashSet::from_iter(vec![2, 3]));
        // assert_eq!(reactor.handlers.get(0).unwrap().S.get(&2).unwrap(), &HashSet::from_iter(vec![5]));
        // assert_eq!(reactor.handlers.get(0).unwrap().S.get(&3).unwrap().is_empty(), true);
        assert_eq!(reactor.handlers.get(0).unwrap().I, HashSet::from_iter(vec![1, 2, 3]));

    }
}
