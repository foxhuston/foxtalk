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

type S<O> = HashMap<O, HashSet<O>>;

pub trait Query<O: Eq + Hash + Clone + Sized + Debug> {
    fn query(o: O) -> bool;
}

pub trait Handler<O: Eq + Hash + Clone + Sized + Debug> {
    fn handle(o: O) -> HashSet<O>;
}

#[allow(non_snake_case)] ////////////////////////////////////////////////
pub struct ReactorHandler<'a, O>
    where O: Eq + Hash + Clone + Sized + Debug
{
    q: dyn Query<O>,
    h: &'a dyn Handler<O>,
    I: HashSet<O>,
    O: HashSet<O>,
    S: S<O>,
    dirty: bool,
}

impl<O: Eq + Hash + Clone + Debug + Sized> ReactorHandler<O> {
    pub fn new() -> Self {
        ReactorHandler {
            I: HashSet::new(),
            O: HashSet::new(),
            S: HashMap::new(),
            dirty: false,
        }
    }

    pub fn add_input(&mut self, input: O) {
        self.I.insert(input);
    }

    pub fn remove_input(&mut self, input: O) {
        self.I.remove(&input);
    }

    pub fn non_aggregating_handle(&mut self) -> (HashSet<O>, S<O>) {
        let s_keys_set = &self.S.keys().clone().collect::<HashSet<&O>>();

        let i_refs: &HashSet<&O> = &self.I.iter().collect::<HashSet<&O>>();

        for x in i_refs.union(&s_keys_set) {
            let needs_inserting = !self.S.contains_key(x);
            let needs_removing = !self.I.contains(x);
            if needs_removing {
                self.S.remove(x);
            }
            if needs_inserting {
                // let results = self.h
                // self.S.insert(x.clone(), HashSet::new());
            }

        };
        // let i_in_s = self.S.iter().map(|(o, _)| o.clone()).collect::<HashSet<O>>();
        // let to_insert = self.I.difference(&i_in_s);
        // let to_remove = i_in_s.difference(&self.I);
        // //
        // // // new_output is everything that was in self.S, unless the key was in to_remove
        // //
        // let new_output_after_removal =
            // .map(|(input, outputSet)| outputSet.clone()).collect::<HashSet<O>>();
        //
        // let insertion_output = to_insert.map(|i| self.h);
        //
        // let new_output: HashSet<O> = new_output_after_removal.union(insertion_output);
        //
        // let new_state = self.S.iter()
        //     .filter(|(input, outputSet)| !to_remove.contains(input))
        //     // .union(insertion_output)
        //     // .map(|(input, outputSet)| (input.clone(), outputSet.clone()))
        //     // .chain(insertion_output)
        //     .collect();
        // //
        // (new_output, new_state)
        todo!()
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
