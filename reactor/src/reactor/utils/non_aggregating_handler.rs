use crate::reactor::reactor_program::Program;
use crate::reactor::ReactorData;
use rustc_hash::{FxHashMap, FxHashSet};

pub trait NonAggregatingProgram<O: ReactorData<Q>, Q> {
    fn query(&self) -> Q;
    fn handle(&mut self, input: O) -> FxHashSet<O>;
}

pub struct NonAggregatingAdapter<O: ReactorData<Q>, Q> {
    h: Box<dyn NonAggregatingProgram<O, Q>>,
    sideband: FxHashMap<O, FxHashSet<O>>
}

impl<O: ReactorData<Q>, Q> NonAggregatingAdapter<O, Q> {
    pub fn new(h: Box<dyn NonAggregatingProgram<O, Q>>) -> Self {
        Self {
            h, sideband: FxHashMap::default()
        }
    }

    fn insert_into_sideband(&mut self, new_values: FxHashMap<O, FxHashSet<O>>) {
        self.sideband.extend(new_values)
    }

    fn remove_from_sideband(&mut self, to_remove: FxHashSet<O>) {
        for i in to_remove {
            self.sideband.remove(&i);
        }
    }
}

unsafe impl<O: ReactorData<Q>, Q> Send for NonAggregatingAdapter<O, Q> {}

impl<O: ReactorData<Q>, Q> Program<O, Q> for NonAggregatingAdapter<O, Q> {
    fn query(&mut self) -> Q {
        self.h.query()
    }

    fn handle(&mut self, input: &FxHashSet<O>) -> FxHashSet<O> {
        let mut newly_inserted: FxHashMap<O, FxHashSet<O>> = FxHashMap::default();
        let mut need_to_remove = FxHashSet::default();

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

    use crate::reactor::query_engine::QueryEngine;
    use crate::reactor::Reactor;
    use std::collections::HashSet;
    use crate::test::{SimpleQuery, SimpleQueryEngine};

    #[test]
    pub fn paper_non_agg_example() {
        struct PaperHandler;
        impl ReactorData<NonAggPaperQuery> for u64{}
        impl NonAggregatingProgram<u64, NonAggPaperQuery> for PaperHandler {
            fn query(&self) -> NonAggPaperQuery {
                NonAggPaperQuery{}
            }
            fn handle(&mut self, o: u64) -> FxHashSet<u64> {
                let mut results = FxHashSet::default();
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

        #[derive(PartialEq, Eq, Clone, Debug, Hash)]
        struct NonAggPaperQuery{}
        impl SimpleQuery<u64> for NonAggPaperQuery {
            fn query(&self, o: &u64) -> bool {
                *o == 1 || *o == 2 || *o == 3
            }
        }

        let paper_query_engine = SimpleQueryEngine::new();

        let mut reactor = Reactor::new(paper_query_engine);

        let handler = Box::new(NonAggregatingAdapter::new(Box::new(PaperHandler)));
        let hid = reactor.add_program(handler);

        reactor.query_engine.insert_program_for_query(NonAggPaperQuery{}, hid.clone());

        reactor.tick();
        // So if we call: 𝗂𝗇𝗌𝖾𝗋𝗍(𝔇0, 𝑜1), then...
        reactor.insert(1);
        assert_eq!(reactor.program_map.get(&hid).unwrap().dirty, true);
        assert_eq!(reactor.program_map.get(&hid).unwrap().I, HashSet::from_iter(vec![1]));
        assert_eq!(reactor.ref_counts.get(&1).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&1).unwrap(), &1);

        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&1).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&1).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&2).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);

        assert_eq!(reactor.program_map.get(&hid).unwrap().dirty, true);
        assert_eq!(reactor.program_map.get(&hid).unwrap().O, HashSet::from_iter(vec![2, 3]));
        // assert_eq!(reactor.handlers.get(0).unwrap().S.get(&1).unwrap(), &HashSet::from_iter(vec![2, 3]));
        assert_eq!(reactor.program_map.get(&hid).unwrap().I, HashSet::from_iter(vec![1, 2, 3]));

        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&1).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&1).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&2).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&5).unwrap(), &1);

        assert_eq!(reactor.program_map.get(&hid).unwrap().dirty, false);
        assert_eq!(reactor.program_map.get(&hid).unwrap().O, HashSet::from_iter(vec![2, 3, 5]));
        // assert_eq!(reactor.handlers.get(0).unwrap().S.get(&1).unwrap(), &HashSet::from_iter(vec![2, 3]));
        // assert_eq!(reactor.handlers.get(0).unwrap().S.get(&2).unwrap(), &HashSet::from_iter(vec![5]));
        // assert_eq!(reactor.handlers.get(0).unwrap().S.get(&3).unwrap().is_empty(), true);
        assert_eq!(reactor.program_map.get(&hid).unwrap().I, HashSet::from_iter(vec![1, 2, 3]));
    }
}