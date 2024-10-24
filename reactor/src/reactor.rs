use std::collections::{HashMap, HashSet};
use std::fmt::Debug;
use std::hash::Hash;
use crate::reactor_handler::ReactorHandler;
//
// pub trait Handler<O: Eq + Hash + Clone + Sized + Debug> {
//     fn handle(&mut self, input: HashSet<&O>) -> HashSet<O>;
// }

/*
 * Reactor has handlers.
 * These handlers live _at most_ as long as Reactor.
 *
 * TupleReactor (specifically) watches its own database (somehow) for tuples that describe handlers
 */


pub struct Reactor<O>
    where O: Eq + Hash + Clone + Sized + Debug {
    pub handlers: Vec<ReactorHandler<O>>,
    pub ref_counts: HashMap<O, u64>
}

impl<O> Reactor<O>
where O: Eq + Hash + Clone + Sized + Debug  {
    pub fn new() -> Self {
        Reactor {
            handlers: Vec::new(),
            ref_counts: HashMap::new()
        }
    }

    pub fn add_handler(&mut self, mut handler: ReactorHandler<O>) {
        for output in &handler.O {
            self.insert(output.clone());
        }

        let qa = &mut handler.qa;
        let all_outputs = self.ref_counts.keys().collect::<HashSet<&O>>();
        for output in all_outputs {
            if qa.query(output) {
                handler.I.insert(output.clone());
            }
        }
        handler.dirty = true;
        self.handlers.push(handler);
    }

    pub fn insert(&mut self, input: O) {
        println!("adding o: {:?}", input);
        if self.ref_counts.contains_key(&input) {
            let count = self.ref_counts.get_mut(&input).unwrap();
            *count += 1;
        } else {

            for handler in self.handlers.iter_mut() {

                let qa = &mut handler.qa;
                if qa.query(&input) && !handler.I.contains(&input) {
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
                let qa = &mut handler.qa;
                let input = &handler.I;
                if !input.is_empty() {
                    let new_output = qa.handle(input);

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
    use crate::reactor_handler::Handler;

    #[test]
    pub fn paper_agg_example() {
        struct PaperHandler;
        impl Handler<u64> for PaperHandler {
            fn query(&mut self, o: &u64) -> bool {
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
}