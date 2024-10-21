// type FFIFn = extern "C" fn();
use std::collections::{HashMap, HashSet};
use std::fmt::Debug;
use std::hash::Hash;
use std::marker::PhantomData;
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

type NonAggHandlerS<O> = HashMap<O, HashSet<O>>;
#[allow(non_snake_case)] ////////////////////////////////////////////////
pub struct ReactorHandler<O, S>
    where O: Eq + Hash + Clone + Sized + Debug
{
    pub q: fn(&O) -> bool,
    pub a: Box<dyn Fn(HashSet<O>, S) -> (HashSet<O>, S)>,
    pub S: S,
    pub I: HashSet<O>,
    pub O: HashSet<O>,
    pub dirty: bool,
}

impl<O: Eq + Hash + Clone + Debug + Sized, S> ReactorHandler<O, S> {
    pub fn new(q: fn(&O) -> bool,
               a: Box<dyn Fn(HashSet<O>, S) -> (HashSet<O>, S)>,
               s: S) -> Self {
        ReactorHandler {
            q,
            a,
            S: s,
            I: HashSet::new(),
            O: HashSet::new(),
            dirty: false,
        }
    }
}

impl<'a, O: Eq + Hash + Clone + Debug + Sized> ReactorHandler<O, NonAggHandlerS<O>> {
    // Hmm. A direct translation from the paper is that this struct should take in an `a: (S X O (input)) -> (S X O (output)),
    // and that the (nonAgg) is a helper function that takes in an h (o -> O (output)) and then generates the `a` using it. So,
    // it needs to close over the h function. However, in Rust, if we want to return a function with closures, we can't use `fn`,
    // instead we need to use one of Fn, FnOnce, or FnMut.

    // However, doing that without changing anything else causes errors. I think we either need to pass in a Box because O isn't sized
    // at compile time, or we need to pass the ref around, and do lifetimes on the struct.
    pub fn non_aggregating_handle(h: impl Fn(&O) -> HashSet<O> + 'a) -> Box<dyn Fn (HashSet<O>, NonAggHandlerS<O>) -> (HashSet<O>, NonAggHandlerS<O>) + 'a> {
        let ret_fn = move |input: HashSet<O>, s: NonAggHandlerS<O> | {

            let mut current_output: HashSet<O> = HashSet::new();
            let mut new_s: NonAggHandlerS<O> = HashMap::new();
            let mut to_remove: HashSet<&O> = HashSet::new();

            for (li, outs) in s.iter() {
                if input.contains(&li) {
                    new_s.insert(li.clone(), outs.clone());
                    current_output.extend(outs.clone());
                } else {
                    to_remove.insert(&li);
                }
            }

            for i in input.iter() {
                if !s.contains_key(i) {
                    let results = h(i);
                    new_s.insert(i.clone(), results.clone());
                    current_output.extend(results.clone());
                }
            }

            (current_output, new_s)

        };
        
        Box::from(ret_fn)
    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    pub fn non_aggregating_handler_works() {
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

        let mut input: HashSet<u64> = HashSet::new();
        input.insert(1);
        let s: NonAggHandlerS<u64> = HashMap::new();
        let (output1, new_s1) = non_agg_handler_box(input.clone(), s);
        input.extend(output1.clone());

        let (output2, new_s2) = non_agg_handler_box(input.clone(), new_s1);
        input.extend(output2.clone());

        assert_eq!(new_s2.get(&1).unwrap(), &HashSet::from_iter(vec![2, 3]));
        assert_eq!(new_s2.get(&2).unwrap(), &HashSet::from_iter(vec![5]));
        assert_eq!(new_s2.get(&3).unwrap().is_empty(), true);


    }
}
