use std::collections::{HashMap, HashSet};
use std::fmt::Debug;
use std::hash::Hash;
use crate::types::{ReactorHandler};
pub struct Reactor<O, S: Clone>
    where O: Eq + Hash + Clone + Sized + Debug { 
    pub handlers: Vec<ReactorHandler<O, S>>,
    pub ref_counts: HashMap<O, u64>
}

impl<O, S: Clone> Reactor<O, S>
where O: Eq + Hash + Clone + Sized + Debug  {
    pub fn new() -> Self {
        Reactor {
            handlers: Vec::new(),
            ref_counts: HashMap::new()
        }
    }

    pub fn add_handler(&mut self, handler: ReactorHandler<O, S>) {
        self.handlers.push(handler);
    }

    pub fn insert(&mut self, input: O) {
        println!("adding o: {:?}", input);
        if self.ref_counts.contains_key(&input) {
            let count = self.ref_counts.get_mut(&input).unwrap();
            *count += 1;
        } else {
            for handler in self.handlers.iter_mut() {
                if (handler.q)(&input) && !handler.I.contains(&input) {
                    handler.I.insert(input.clone());
                    handler.dirty = true;
                }
            }
            self.ref_counts.insert(input.clone(), 1);
        }
    }

    pub fn remove(&mut self, input: O) {
        println!("adding o: {:?}", input);
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
        let mut need_to_insert_total = HashSet::new();
        let mut need_to_remove_total = HashSet::new();
        for handler in self.handlers.iter_mut() {
            if handler.dirty {
                let (new_output, new_state) = (handler.a)(handler.I.clone(), handler.S.clone());
                
                let need_to_insert = new_output.difference(&handler.O).cloned().collect::<HashSet<O>>();
                for i in need_to_insert.iter() {
                    need_to_insert_total.insert(i.clone());
                }
                let need_to_remove = handler.O.difference(&new_output).cloned().collect::<HashSet<O>>();
                for i in need_to_remove.iter() {
                    need_to_remove_total.insert(i.clone());
                }
                handler.O = new_output;
                handler.S = new_state;
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
    use std::collections::HashSet;
    use super::*;

    #[test]
    pub fn paper_agg_example() {

        fn paper_query(other: &u64) -> bool {
            *other > 5 && *other < 20
        }
        fn agg(o: HashSet<u64>, s: ()) -> (HashSet<u64>, ()) {
            let col: u64 = o.iter().sum();
            let mut out = HashSet::new();
            out.insert(col);
            (out, ())
        }
        
        let boxed_agg = Box::new(agg);

        let handler = ReactorHandler::new(paper_query, boxed_agg, ());

        let mut reactor = Reactor::new();
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
        fn handle(o: &u64) -> HashSet<u64> {
            match o {
                1 => {
                    let mut results = HashSet::new();
                    results.insert(2);
                    results.insert(3);
                    results
                }
                2 => {
                    let mut results = HashSet::new();
                    results.insert(5);
                    results
                }
                _ => HashSet::new()
            }
        }
        
        let non_agg_handler_box = ReactorHandler::non_aggregating_handle(handle);
        
        let non_agg_handler = non_agg_handler_box;

        fn paper_query(other: &u64) -> bool {
            *other == 1 || *other == 2 || *other == 3
        }
        
        let handler = ReactorHandler::new(paper_query, non_agg_handler, HashMap::new());

        let mut reactor = Reactor::new();
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
        assert_eq!(reactor.handlers.get(0).unwrap().S.get(&1).unwrap(), &HashSet::from_iter(vec![2, 3]));
        assert_eq!(reactor.handlers.get(0).unwrap().I, HashSet::from_iter(vec![1, 2, 3]));

        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&1).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&1).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&2).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&5).unwrap(), &1);

        assert_eq!(reactor.handlers.get(0).unwrap().dirty, false);
        assert_eq!(reactor.handlers.get(0).unwrap().O, HashSet::from_iter(vec![2, 3, 5]));
        assert_eq!(reactor.handlers.get(0).unwrap().S.get(&1).unwrap(), &HashSet::from_iter(vec![2, 3]));
        assert_eq!(reactor.handlers.get(0).unwrap().S.get(&2).unwrap(), &HashSet::from_iter(vec![5]));
        assert_eq!(reactor.handlers.get(0).unwrap().S.get(&3).unwrap().is_empty(), true);
        assert_eq!(reactor.handlers.get(0).unwrap().I, HashSet::from_iter(vec![1, 2, 3]));
        
    }
}