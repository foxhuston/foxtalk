use crate::reactor::Handler;
use std::collections::{HashMap, HashSet};
use std::fmt::Debug;
use std::hash::Hash;

pub struct PureHandler <'a, O: Eq + Hash + Clone + Sized + Debug>{
    aggregate: &'a mut dyn FnMut(HashSet<&O>) -> HashSet<O>
}

impl<'a, O: Eq + Hash + Clone + Sized + Debug> PureHandler<'a, O> {
    pub fn new(aggregate: &'a mut dyn FnMut(HashSet<&O>) -> HashSet<O>) -> Self {
        PureHandler {
            aggregate
        }
    }
}

pub struct NonAggHandler <'a, O: Eq + Hash + Clone + Sized + Debug> {
    sideband: NonAggHandlerS<O>,
    handle: &'a mut dyn FnMut(&O) -> HashSet<O>
}
impl<'a, O: Eq + Hash + Clone + Sized + Debug> NonAggHandler<'a, O> {
    fn insert_into_sideband(&mut self, new_values: HashMap<O, HashSet<O>>) {
        self.sideband.extend(new_values)
    }
    fn remove_from_sideband(&mut self, to_remove: HashSet<O>) {
        for i in to_remove {
            self.sideband.remove(&i);
        }
    }
    pub fn new(handle: &'a mut dyn FnMut(&O) -> HashSet<O>) -> Self {
        NonAggHandler {
            sideband: HashMap::new(),
            handle
        }
    }
}
impl<'a, O: Eq + Hash + Clone + Sized + Debug> Handler<O> for PureHandler<'a, O> {
    fn handle(&mut self, input: HashSet<&O>) -> HashSet<O> {
        (self.aggregate)(input)
    }
}

impl<'a, O: Eq + Hash + Clone + Sized + Debug> Handler<O> for NonAggHandler<'a, O> {
    fn handle(&mut self, input: HashSet<&O>) -> HashSet<O> {
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

pub type NonAggHandlerS<O> = HashMap<O, HashSet<O>>;
#[allow(non_snake_case)] ////////////////////////////////////////////////

pub trait ReactorHandle<O> {
    fn q(&mut self, o: &O) -> bool;
    fn a(&mut self, o: &HashSet<&O>) -> HashSet<O>;
}

pub struct ReactorHandler<'a, O>
    where O: Eq + Hash + Clone + Sized + Debug
{
    pub qa: Box<dyn ReactorHandle<O> +'a >,
    pub I: HashSet<O>,
    pub O: HashSet<O>,
    pub dirty: bool,
}

impl<'a, O: Eq + Hash + Clone + Debug + Sized> ReactorHandler<'a, O> {
    pub fn new(qa: Box<dyn ReactorHandle<O> +'a>) -> Self {
        ReactorHandler {
            qa,
            I: HashSet::new(),
            O: HashSet::new(),
            dirty: false,
        }
    }
}