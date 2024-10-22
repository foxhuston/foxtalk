// type FFIFn = extern "C" fn();
use std::collections::{HashMap, HashSet};
use std::fmt::Debug;
use std::hash::Hash;
use crate::reactor::Handler;
//
//
// trait Lib<'a> {
//     fn init(self) -> FFIFn;
//     fn teardown(self) -> FFIFn;
//     fn handle(self) -> FFIFn;
//     fn free(self) ->FFIFn;
//     fn buffer(self) -> &'a mut [u8];
// }
//


pub struct PureHandler <O: Eq + Hash + Clone + Sized + Debug>{
    aggregate: Box<dyn Fn(HashSet<&O>) -> HashSet<O>>
}

impl<O: Eq + Hash + Clone + Sized + Debug> PureHandler<O> {
    pub fn new(aggregate: Box<dyn Fn(HashSet<&O>) -> HashSet<O>>) -> Self {
        PureHandler {
            aggregate
        }
    }
}

pub struct NonAggHandler <O: Eq + Hash + Clone + Sized + Debug> {
    sideband: NonAggHandlerS<O>,
    handle: Box<dyn Fn(&O) -> HashSet<O>>
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
    pub fn new(handle: Box<dyn Fn(&O) -> HashSet<O>>) -> Self {
        NonAggHandler {
            sideband: HashMap::new(),
            handle
        }
    }
}
impl<O: Eq + Hash + Clone + Sized + Debug> Handler<O> for PureHandler<O> {
    fn handle(&mut self, input: HashSet<&O>) -> HashSet<O> {
        (self.aggregate)(input)
    }
}

impl<O: Eq + Hash + Clone + Sized + Debug> Handler<O> for NonAggHandler<O> {
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
pub struct ReactorHandler<O>
    where O: Eq + Hash + Clone + Sized + Debug
{
    pub q: fn(&O) -> bool,
    pub a: Box<dyn Handler<O>>,
    pub I: HashSet<O>,
    pub O: HashSet<O>,
    pub dirty: bool,
}

impl<O: Eq + Hash + Clone + Debug + Sized> ReactorHandler<O> {
    pub fn new(q: fn(&O) -> bool,
               a: Box<dyn Handler<O>>) -> Self {
        ReactorHandler {
            q,
            a,
            I: HashSet::new(),
            O: HashSet::new(),
            dirty: false,
        }
    }
}
//
// impl<'a, O: Eq + Hash + Clone + Debug + Sized> ReactorHandler<O, NonAggHandlerS<O>> {
//     // Hmm. A direct translation from the paper is that this struct should take in an `a: (S X O (input)) -> (S X O (output)),
//     // and that the (nonAgg) is a helper function that takes in an h (o -> O (output)) and then generates the `a` using it. So,
//     // it needs to close over the h function. However, in Rust, if we want to return a function with closures, we can't use `fn`,
//     // instead we need to use one of Fn, FnOnce, or FnMut.
//
//     // However, doing that without changing anything else causes errors. I think we either need to pass in a Box because O isn't sized
//     // at compile time, or we need to pass the ref around, and do lifetimes on the struct.
//     pub fn non_aggregating_handle(h: impl Fn(&O) -> HashSet<O> + 'a) -> Box<dyn Fn (HashSet<O>, NonAggHandlerS<O>) -> (HashSet<O>, NonAggHandlerS<O>) + 'a> {
//         let ret_fn = move |input: HashSet<O>, s: NonAggHandlerS<O> | {
//
//             let mut current_output: HashSet<O> = HashSet::new();
//             let mut new_s: NonAggHandlerS<O> = HashMap::new();
//             let mut to_remove: HashSet<&O> = HashSet::new();
//
//             for (li, outs) in s.iter() {
//                 if input.contains(&li) {
//                     new_s.insert(li.clone(), outs.clone());
//                     current_output.extend(outs.clone());
//                 } else {
//                     to_remove.insert(&li);
//                 }
//             }
//
//             for i in input.iter() {
//                 if !s.contains_key(i) {
//                     let results = h(i);
//                     new_s.insert(i.clone(), results.clone());
//                     current_output.extend(results.clone());
//                 }
//             }
//
//             (current_output, new_s)
//
//         };
//
//         Box::from(ret_fn)
//     }
