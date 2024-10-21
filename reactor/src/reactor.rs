use std::collections::HashMap;
use std::fmt::Debug;
use std::hash::Hash;
use crate::types::{ReactorHandler};
pub struct Reactor<O, S>
    where O: Eq + Hash + Clone + Sized + Debug { 
    pub handlers: Vec<ReactorHandler<O, S>>,
    pub ref_counts: HashMap<O, u64>
}

impl<O, S> Reactor<O, S>
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
        println!("Todo: Actually add")
    }

    pub fn remove(&mut self, input: O) {
        println!("adding o: {:?}", input);
        println!("Todo: Actually add")
    }

    pub fn tick(&mut self) {
        println!("tick tick tick");
        println!("Todo: tick")
    }


}

#[cfg(test)]
mod tests {
    use std::collections::HashSet;
    use super::*;

    #[test]
    pub fn paper_agg_example() {
        
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
        
        let non_agg_handler = *non_agg_handler_box;

        fn paper_query(other: &u64) -> bool {
            *other == 1 || *other == 2
        }
        
        let handler = ReactorHandler::new(paper_query, non_agg_handler, HashSet::new());

        let mut reactor = Reactor::new();
        reactor.add_handler(handler);

        reactor.tick();
        // So if we call: 𝗂𝗇𝗌𝖾𝗋𝗍(𝔇0, 𝑜1), then...
        reactor.insert(1);
        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&1).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&1).unwrap(), &1);
        
        
        
    }
}