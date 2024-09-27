use std::collections::{HashMap, HashSet};
use crate::db::{Db, DbIndex};
use crate::tuple::Tuple;
use crate::when::{When};

struct Reactor {
    handlers: Vec<Box<dyn When>>,
    db: Db,

    handler_provenance: (), // TODO: How do I even express this in Rust?????
    tuple_provenance: DbIndex<Tuple, Tuple>
}

impl Reactor {
    fn new() -> Reactor {
        Reactor {
            db: Db::new(),
            handlers: Vec::new(),

            handler_provenance: (),
            tuple_provenance: DbIndex::new()
        }
    }

    pub fn add_handler(&mut self, handler: Box<dyn When>) {
        self.handlers.push(handler);
    }

    pub fn claim(&mut self, tuple: Tuple) {
        self.db.claim(tuple);
    }

    pub fn remove_claim(&mut self, tuple: Tuple) {
        let mut work_queue: Vec<Tuple> = Vec::new();

        match self.tuple_provenance.get_mut(&tuple) {
            None => {}
            Some(generated_tuples) => {
                for tup in generated_tuples.drain() {
                    work_queue.push(tup);
                }
            }
        }

        for tup in work_queue {
            self.remove_claim(tup);
        }

        self.db.remove_claim(tuple);
    }

    pub fn tick(&mut self) {
        // Doing this in stages because otherwise `self` is doubly-mutably borrowed...
        // Which like... fine, whatever. The gist is that we have a locally mutable hash set
        // into which we place tuples generated across all handlers. Once we've done that,
        // the mutably-borrowed scope is ended (at the closing `}` of the for-loop),
        // and we can re-borrow ourselves to insert the claims into the db.
        let mut changeQueue: HashSet<(Tuple, Tuple)> = HashSet::new();

        for h in self.handlers.iter_mut() {
            let results = self.db.query(h.get_query());

            for result in results {
                let r = result.clone();
                h.handle(&mut |t| {
                    println!("Inserting {r:?} --> {t:?} into changeQueue...");
                    changeQueue.insert((r.clone(), t));
                }, result);
            }
        }

        for (prov, c) in changeQueue.drain() {
            println!("Recording {c:?} into reactor db");

            match self.tuple_provenance.get_mut(&prov) {
                None => {
                    let mut hs = HashSet::new();
                    hs.insert(c.clone());
                    self.tuple_provenance.insert(prov, hs);
                }
                Some(mut set) => {
                    set.insert(c.clone());
                }
            }

            self.db.claim(c);
        }
    }
}

///// TESTS ////////////////////////////////////////////////////////////////////

#[cfg(test)]
mod tests {
    use super::*;
    use crate::when::{When};
    use crate::tuple::TupleNoun::*;
    use crate::query::Query;

    ///// WHEN-HANDLER TESTS ///////////////////////////////////////////////////

    struct HuskyHandler {}

    impl When for HuskyHandler {
        fn get_query(&self) -> Query {
            // When /who/ is a husky:
            Query {
                subject: None,
                predicate: Some("is a".to_string()),
                object: Some(Str("husky".to_string())),
            }
        }

        fn handle(&mut self, wish: &mut dyn FnMut(Tuple), results: Tuple) -> () {
            // Wish (who) is highlighted blue.
            wish(Tuple {
                subject: results.subject,
                predicate: "is highlighted".to_string(),
                object: Str("blue".to_string()),
            })
        }
    }

    struct HighlightHandler {}
    impl When for HighlightHandler {
        fn get_query(&self) -> Query {
            // When /someone/ is highlighted /color/:
            Query {
                subject: None,
                predicate: Some("is highlighted".to_string()),
                object: None
            }
        }

        fn handle(&mut self, wish: &mut dyn FnMut(Tuple), results: Tuple) -> () {
            // Wish (someone) debug_illuminated (color).
            wish(Tuple {
                subject: results.subject,
                predicate: "debug_illuminated".to_string(),
                object: results.object,
            })
        }
    }

    #[test]
    fn already_registered_handlers_trigger() {
        let mut reactor = Reactor::new();
        let handler = Box::new(HuskyHandler {});

        reactor.add_handler(handler);

        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        reactor.claim(tuple.clone());

        reactor.tick();

        assert_eq!(reactor.db.query(Query::from_strs(Some("lexi"), None, None)).iter().count(), 2);
    }

    #[test]
    fn newly_registered_handlers_trigger() {
        let mut reactor = Reactor::new();

        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        reactor.claim(tuple.clone());

        let handler = Box::new(HuskyHandler {});
        reactor.add_handler(handler);

        reactor.tick();

        assert_eq!(reactor.db.query(Query::from_strs(Some("lexi"), None, None)).iter().count(), 2);
    }

    #[test]
    fn removed_originial_tuples_remove_generated_tuples() {
        let mut reactor = Reactor::new();

        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        reactor.claim(tuple.clone());

        let handler = Box::new(HuskyHandler {});
        reactor.add_handler(handler);

        reactor.tick();

        assert_eq!(reactor.db.query(Query::from_strs(Some("lexi"), Some("is highlighted"), Some("blue"))).iter().count(), 1);

        reactor.remove_claim(tuple);
        reactor.tick();

        assert_eq!(reactor.db.query(Query::from_strs(Some("lexi"), Some("is highlighted"), Some("blue"))).iter().count(), 0);
    }

    #[test]
    fn removed_originial_tuples_transitively_remove_generated_tuples() {
        let mut reactor = Reactor::new();

        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        reactor.claim(tuple.clone());

        reactor.add_handler(Box::new(HuskyHandler {}));
        reactor.add_handler(Box::new(HighlightHandler {}));

        reactor.tick();
        reactor.tick();

        assert_eq!(reactor.db.query(Query::from_strs(Some("lexi"), Some("is highlighted"), Some("blue"))).iter().count(), 1);
        assert_eq!(reactor.db.query(Query::from_strs(Some("lexi"), Some("debug_illuminated"), Some("blue"))).iter().count(), 1);

        reactor.remove_claim(tuple);
        reactor.tick();

        assert_eq!(reactor.db.query(Query::from_strs(Some("lexi"), Some("is highlighted"), Some("blue"))).iter().count(), 0);
        assert_eq!(reactor.db.query(Query::from_strs(Some("lexi"), Some("debug_illuminated"), Some("blue"))).iter().count(), 0);
    }
}