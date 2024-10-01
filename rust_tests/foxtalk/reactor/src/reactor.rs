use tuple_db::db::{Db, DbIndex};
use crate::tuple::Tuple;
use crate::when::When;
use std::collections::{HashMap, HashSet};

use uuid::Uuid;

type Handler = Box<dyn When>;

#[repr(transparent)]
#[derive(PartialEq, Eq, Hash, Clone)]
pub struct HandlerId(Uuid);

pub struct Reactor<'a> {
    // This uses UUID keys so that we can quickly find handlers
    // without needing the handlers themselves to be hashable.
    handlers: HashMap<HandlerId, Handler>,

    // TODO: UNPUB
    pub db: Db<'a>,

    handler_provenance: DbIndex<HandlerId, &'a Tuple>,
    tuple_provenance: DbIndex<&'a Tuple, &'a Tuple>,

    // Tuple Triggered handler (TODO: with query?)
    handler_ran_for_tuple: DbIndex<&'a Tuple, HandlerId>,
}

impl Reactor<'_> {
    pub fn new<'a>() -> Reactor<'a> {
        Reactor {
            db: Db::new(),
            handlers: HashMap::new(),

            handler_provenance: DbIndex::new(),
            tuple_provenance: DbIndex::new(),

            handler_ran_for_tuple: DbIndex::new()

        }
    }

    pub fn add_handler(&mut self, handler: Box<dyn When>) -> HandlerId {
        let id = HandlerId(Uuid::new_v4());
        self.handlers.insert(id.clone(), handler);
        id
    }

    pub fn remove_handler(&mut self, id: HandlerId) {
        match self.handler_provenance.remove_all_by_key(&id) {
            None => {}
            Some(tuples) => {
                for tuple in tuples {
                    self.remove_claim(tuple);
                }
            }
        }

        self.handlers.remove(&id);
    }

    pub fn claim(&mut self, tuple: Tuple) {
        self.db.claim(tuple);
    }

    pub fn remove_claim(&mut self, tuple: Tuple) {
        let mut work_queue: Vec<Tuple> = Vec::new();

        // Clean up provenance / caches.
        self.handler_ran_for_tuple.remove_all_by_key(&tuple);
        self.handler_provenance.remove_all_by_value(&tuple);
        self.tuple_provenance.remove_all_by_value(&tuple);

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
        // into which we place tuple generated across all handlers. Once we've done that,
        // the mutably-borrowed scope is ended (at the closing `}` of the for-loop),
        // and we can re-borrow ourselves to insert the claims into the db.
        let mut change_queue: HashSet<(Tuple, HandlerId, Tuple)> = HashSet::new();

        for (hid, h) in self.handlers.iter_mut() {
            let results = self.db.query(h.get_query());

            for query_result in results {
                if !self.handler_ran_for_tuple.contains(&query_result, hid) {
                    self.handler_ran_for_tuple.insert(query_result.clone(), hid.clone());
                    let qr = query_result.clone();

                    let wishes = h.handle(query_result);
                    wishes.into_iter()
                        .map(|wished_tuple| (qr.clone(), hid.clone(), wished_tuple))
                        .for_each(|t| {
                            change_queue.insert(t);
                            ()
                        });
                }
            }
        }

        for (prov, hid, c) in change_queue.drain() {
            println!("Recording {c:?} into reactor db");

            self.handler_provenance.insert(hid, c.clone());
            self.tuple_provenance.insert(prov, c.clone());
            self.db.claim(c);
        }
    }
}

///// TESTS ////////////////////////////////////////////////////////////////////

#[cfg(test)]
mod tests {
    use super::*;
    use crate::tuple::TupleNoun::*;
    use crate::when::When;
    use crate::tuple::test_helpers::*;
    use crate::tuple::TupleNoun;
    ///// WHEN-HANDLER TESTS ///////////////////////////////////////////////////

    struct HuskyHandler {}

    impl When for HuskyHandler {
        fn get_query(&self) -> Tuple {
            // When /who/ is a husky:
            mk_query(None, Some("is a"), Some("husky"))
        }

        fn handle(&mut self, results: Tuple) -> Vec<Tuple> {
            // Wish (who) is highlighted blue.
            vec![Tuple {
                subject: results.subject,
                predicate: TupleNoun::from_str("is highlighted"),
                object: TupleNoun::from_str("blue"),
            }]
        }
    }

    struct HighlightHandler {}
    impl When for HighlightHandler {
        fn get_query(&self) -> Tuple {
            // When /someone/ is highlighted /color/:
            mk_query(None, Some("is highlighted"), None)
        }

        fn handle(&mut self, results: Tuple) -> Vec<Tuple> {
            // Wish (someone) debug_illuminated (color).
            vec![Tuple {
                subject: results.subject,
                predicate: TupleNoun::from_str("debug_illuminated"),
                object: results.object,
            }]
        }
    }

    #[test]
    fn already_registered_handlers_trigger() {
        let mut reactor = Reactor::new();
        let handler = Box::new(HuskyHandler {});

        reactor.add_handler(handler);

        let tuple = mk_tuple("lexi", "is a", "husky");
        reactor.claim(tuple.clone());

        reactor.tick();

        assert_eq!(reactor.db.query(mk_query(Some("lexi"), None, None)).iter().count(), 2);
    }

    #[test]
    fn newly_registered_handlers_trigger() {
        let mut reactor = Reactor::new();

        let tuple = mk_tuple("lexi", "is a", "husky");
        reactor.claim(tuple.clone());

        let handler = Box::new(HuskyHandler {});
        reactor.add_handler(handler);

        reactor.tick();

        assert_eq!(reactor.db.query(mk_query(Some("lexi"), None, None)).iter().count(), 2);
    }

    #[test]
    fn removed_originial_tuples_remove_generated_tuples() {
        let mut reactor = Reactor::new();

        let tuple = mk_tuple("lexi", "is a", "husky");
        reactor.claim(tuple.clone());

        let handler = Box::new(HuskyHandler {});
        reactor.add_handler(handler);

        reactor.tick();

        assert_eq!(reactor.db.query(mk_query(Some("lexi"), Some("is highlighted"), Some("blue"))).iter().count(), 1);

        reactor.remove_claim(tuple);
        reactor.tick();

        assert_eq!(reactor.db.query(mk_query(Some("lexi"), Some("is highlighted"), Some("blue"))).iter().count(), 0);
    }

    #[test]
    fn removed_originial_tuples_transitively_remove_generated_tuples() {
        let mut reactor = Reactor::new();

        let tuple = mk_tuple("lexi", "is a", "husky");
        reactor.claim(tuple.clone());

        reactor.add_handler(Box::new(HuskyHandler {}));
        reactor.add_handler(Box::new(HighlightHandler {}));

        reactor.tick();
        reactor.tick();

        assert_eq!(reactor.db.query(mk_query(Some("lexi"), Some("is highlighted"), Some("blue"))).iter().count(), 1);
        assert_eq!(reactor.db.query(mk_query(Some("lexi"), Some("debug_illuminated"), Some("blue"))).iter().count(), 1);

        reactor.remove_claim(tuple);
        reactor.tick();

        assert_eq!(reactor.db.query(mk_query(Some("lexi"), Some("is highlighted"), Some("blue"))).iter().count(), 0);
        assert_eq!(reactor.db.query(mk_query(Some("lexi"), Some("debug_illuminated"), Some("blue"))).iter().count(), 0);
    }

    #[test]
    fn removed_originial_handlers_transitively_remove_generated_tuples() {
        let mut reactor = Reactor::new();

        let tuple = mk_tuple("lexi", "is a", "husky");
        reactor.claim(tuple.clone());

        let husky_handler_id = reactor.add_handler(Box::new(HuskyHandler {}));
        reactor.add_handler(Box::new(HighlightHandler {}));

        reactor.tick();
        reactor.tick();

        assert_eq!(reactor.db.query(mk_query(Some("lexi"), Some("is highlighted"), Some("blue"))).iter().count(), 1);
        assert_eq!(reactor.db.query(mk_query(Some("lexi"), Some("debug_illuminated"), Some("blue"))).iter().count(), 1);

        reactor.remove_handler(husky_handler_id);
        reactor.tick();

        assert_eq!(reactor.db.query(mk_query(Some("lexi"), Some("is highlighted"), Some("blue"))).iter().count(), 0);
        assert_eq!(reactor.db.query(mk_query(Some("lexi"), Some("debug_illuminated"), Some("blue"))).iter().count(), 0);
    }
}