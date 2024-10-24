
use std::collections::{HashMap, HashSet};
use std::fmt::Debug;
use std::hash::Hash;
use crate::reactor_handler::Handler;

pub type NonAggHandlerS<O> = HashMap<O, HashSet<O>>;
pub struct PureHandler <O: Eq + Hash + Clone + Sized + Debug>{
    aggregate: Box<dyn FnMut(HashSet<&O>) -> HashSet<O>>
}

impl<O: Eq + Hash + Clone + Sized + Debug> PureHandler<O> {
    pub fn new(aggregate: Box<dyn FnMut(HashSet<&O>) -> HashSet<O>>) -> Self {
        PureHandler {
            aggregate
        }
    }
    pub fn handle(&mut self, input: HashSet<&O>) -> HashSet<O> {
        (self.aggregate)(input)
    }
}

pub struct NonAggHandler <O: Eq + Hash + Clone + Sized + Debug> {
    sideband: NonAggHandlerS<O>,
    handle: Box<dyn FnMut(&O) -> HashSet<O>>
}
impl<O: Eq + Hash + Clone + Sized + Debug> NonAggHandler<O> {
    fn insert_into_sideband(&mut self, new_values: HashMap<O, HashSet<O>>) {
        self.sideband.extend(new_values)
    }
    fn remove_from_sideband(&mut self, to_remove: HashSet<O>) {
        for i in to_remove {
            self.sideband.remove(&i);
        }
    }
    pub fn new(handle: Box<dyn FnMut(&O) -> HashSet<O>>) -> Self {
        NonAggHandler {
            sideband: HashMap::new(),
            handle
        }
    }
    pub fn handle(&mut self, input: HashSet<&O>) -> HashSet<O> {
        let keys = self.sideband.keys().collect::<HashSet<&O>>();
        let all_inputs = keys.union(&input).collect::<HashSet<&&O>>();
        let mut newly_inserted: HashMap<O, HashSet<O>> = HashMap::new();
        let mut need_to_remove = HashSet::new();
        for &i in all_inputs {
            if !self.sideband.contains_key(i) {
                let results = (self.handle)(i);
                newly_inserted.insert(i.clone(), results);
            }
            if !input.contains(i) {
                need_to_remove.insert(i.clone());
            }
        }
        self.remove_from_sideband(need_to_remove);
        self.insert_into_sideband(newly_inserted);

        self.sideband.values().flat_map(|x| x.clone()).collect()
    }
}