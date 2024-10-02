use tuple_db::db::{Db, DbIndex};
use tuple_db::tuple::Tuple;
use crate::when::When;
use std::collections::{HashMap, HashSet};
use std::sync::Arc;
use uuid::Uuid;

type Handler = Box<dyn When>;

#[repr(transparent)]
#[derive(PartialEq, Eq, Hash, Clone)]
pub struct HandlerId(Uuid);

pub struct Reactor {
    // This uses UUID keys so that we can quickly find handlers
    // without needing the handlers themselves to be hashable.
    handlers: HashMap<HandlerId, Handler>,

    // TODO: UNPUB
    pub db: Db,

    handler_provenance: DbIndex<HandlerId, Arc<Tuple>>,
    tuple_provenance: DbIndex<Arc<Tuple>, Arc<Tuple>>,

    // Tuple Triggered handler (TODO: with query?)
    handler_ran_for_tuple: DbIndex<Arc<Tuple>, HandlerId>,
}

impl Reactor {
    pub fn new<'a>() -> Reactor {
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

    pub fn claim(&mut self, tuple: Arc<Tuple>) {
        self.db.claim(tuple);
    }

    pub fn remove_claim(&mut self, tuple: Arc<Tuple>) {
        let mut work_queue: Vec<Arc<Tuple>> = Vec::new();

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
        let mut change_queue: HashSet<(Arc<Tuple>, HandlerId, Arc<Tuple>)> = HashSet::new();

        for (hid, h) in self.handlers.iter_mut() {
            let results = self.db.query(h.get_query());

            for query_result in results {
                if !self.handler_ran_for_tuple.contains(&query_result, hid) {
                    self.handler_ran_for_tuple.insert(query_result.clone(), hid.clone());
                    let qr = query_result.clone();

                    let wishes = h.handle(query_result);
                    wishes.into_iter()
                        .map(|wished_tuple| {
                            let qrc = qr.clone();
                            let hidc = hid.clone();
                            (qrc, hidc, wished_tuple)
                        })
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
    use tuple_db::tuple::test_helpers::mk_query;
    use tuple_db::tuple::TupleNoun;
    use super::*;
    use tuple_db::tuple::TupleNoun::*;
    use crate::when::When;
    use tuple_db::tuple::test_helpers::*;
    ///// WHEN-HANDLER TESTS ///////////////////////////////////////////////////

    struct HuskyHandler {}

    impl When for HuskyHandler {
        fn get_query(&self) -> Arc<Tuple> {
            // When /who/ is a husky:
            mk_query(None, Some("is a"), Some("husky"))
        }

        fn handle(&mut self, results: Arc<Tuple>) -> Vec<Arc<Tuple>> {
            // Wish (who) is highlighted blue.
            vec![Arc::new(Tuple {
                subject: results.subject.clone(),
                predicate: Arc::new(TupleNoun::from_str("is highlighted")),
                object: Arc::new(TupleNoun::from_str("blue")),
            })]
        }
    }

    struct HighlightHandler {}
    impl When for HighlightHandler {
        fn get_query(&self) -> Arc<Tuple> {
            // When /someone/ is highlighted /color/:
            mk_query(None, Some("is highlighted"), None)
        }

        fn handle(&mut self, results: Arc<Tuple>) -> Vec<Arc<Tuple>> {
            // Wish (someone) debug_illuminated (color).
            vec![Arc::new(Tuple {
                subject: results.subject.clone(),
                predicate: Arc::new(TupleNoun::from_str("debug_illuminated")),
                object: results.object.clone(),
            })]
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