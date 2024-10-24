use std::collections::{HashMap, HashSet};
use std::hash::Hash;
use crate::reactor::reactor_handler::Handler;

pub trait NonAggregatingHandler<O> {
    fn query(&mut self, o: &O) -> bool;
    fn handle(&mut self, input: O) -> HashSet<O>;
}

pub struct NonAggregatingAdapter<O> {
    h: Box<dyn NonAggregatingHandler<O>>,
    sideband: HashMap<O, HashSet<O>>
}

impl<O: Eq + Hash> NonAggregatingAdapter<O> {
    pub fn new(h: Box<dyn NonAggregatingHandler<O>>) -> Self {
        Self {
            h, sideband: HashMap::new()
        }
    }

    fn insert_into_sideband(&mut self, new_values: HashMap<O, HashSet<O>>) {
        self.sideband.extend(new_values)
    }

    fn remove_from_sideband(&mut self, to_remove: HashSet<O>) {
        for i in to_remove {
            self.sideband.remove(&i);
        }
    }
}

impl<O: Eq + Hash + Clone> Handler<O> for NonAggregatingAdapter<O> {
    fn query(&mut self, o: &O) -> bool {
        self.h.query(o)
    }

    fn handle(&mut self, input: &HashSet<O>) -> HashSet<O> {
        let mut newly_inserted: HashMap<O, HashSet<O>> = HashMap::new();
        let mut need_to_remove = HashSet::new();

        for i in input {
            if !self.sideband.contains_key(&i) {
                let results = self.h.handle(i.clone());
                newly_inserted.insert(i.clone(), results);
            }
        }

        for s in self.sideband.keys() {
            if !input.contains(&s) {
                need_to_remove.insert(s.clone());
            }
        }

        self.remove_from_sideband(need_to_remove);
        self.insert_into_sideband(newly_inserted);

        self.sideband.values().flat_map(|x| x.clone()).collect()
    }
}

#[cfg(test)]
mod test {
    use super::*;

    use std::collections::HashSet;
    use crate::reactor::Reactor;

    #[test]
    pub fn paper_non_agg_example() {
        struct PaperHandler;
        impl NonAggregatingHandler<u64> for PaperHandler {
            fn query(&mut self, o: &u64) -> bool {
                *o == 1 || *o == 2 || *o == 3
            }
            fn handle(&mut self, o: u64) -> HashSet<u64> {
                let mut results = HashSet::new();
                match o {
                    1 => {
                        results.insert(2);
                        results.insert(3);
                    }
                    2 => {
                        results.insert(5);
                    }
                    _ => {}
                }
                results
            }
        }

        let mut reactor = Reactor::new();
        let handler = Box::new(NonAggregatingAdapter::new(Box::new(PaperHandler)));
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