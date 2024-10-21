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
    q: fn(&O) -> bool,
    a: fn(HashSet<O>, S) -> (HashSet<O>, S),
    S: S,
    I: HashSet<O>,
    O: HashSet<O>,
    dirty: bool,
}

impl<O: Eq + Hash + Clone + Debug + Sized, S> ReactorHandler<O, S> {
    pub fn new(q: fn(&O) -> bool,
               a: fn(HashSet<O>, S) -> (HashSet<O>, S),
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

    pub fn add_input(&mut self, input: O) {
        self.I.insert(input);
    }

    pub fn remove_input(&mut self, input: O) {
        self.I.remove(&input);
    }

    fn remove_output(&mut self, outputs: Vec<&O>) {
        for x in outputs {
            self.O.remove(&x);
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
    pub fn non_aggregating_handle(h: fn(&O) -> HashSet<O>) -> Box<(dyn FnOnce (HashSet<O>, NonAggHandlerS<O>) -> (HashSet<O>, NonAggHandlerS<O>))> {
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
                    let results = h(move i);
                    new_s.insert(i.clone(), results.clone());
                    current_output.extend(results.clone());
                }
            }

            (current_output, new_s)

        };

        let ret_box = Box::new(ret_fn);

        ret_box

    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    pub fn can_create_handler_structs() {

        // The interface here should be that:

        // Create a struct that contains what you want to query about.
        // Essentially, something that relates to O

        // For triples, this would contain a subject, predicate, and object that could be Queries
        struct GtQuery {
            gt: u64,
        }

        // Then, we have to show how our object actually interacts with O. If this were triples, then
        // this might be something like:
        /*
            (Q.subject == other.subject || q.subject == QUERY) &&
            (Q.pred == other.pred || q.pred == QUERY) &&
            (Q.object == other.object || q.object == QUERY) &&
         */
        // For test, we create a simple greater than query

        struct SbInt {
            i: u64
        }

        impl GtQuery {
            fn matches(self, other: u64) -> bool {
                self.gt > other
            }
        }


        let query10 = GtQuery { gt: 10 };
        let query100 = GtQuery { gt: 100 };
        // let handler10 = ReactorHandler::new(query10);

        let number_needing_handling_10:u64  = 16;
        let number_needing_handling_100:u64  = 160; // also handled by 10
        let number_not_needing_handling:u64  = 6;

        //
        //
        // let handler10 = ReactorHandler {
        //     I: HashSet::new(),
        //     O: HashSet::new(),
        //     S: HashSet::new(),
        //     q: |o| query10.matches(o),
        //     h: |o, s| {
        //
        //         (o, 10)
        //     }
        // };


    }
}
