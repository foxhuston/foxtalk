use crate::tuple::{Tuple, TupleNoun};

use std::collections::{HashMap, HashSet};

type DbIndex<K, V> = HashMap<K, HashSet<V>>;

pub struct Db {
    // We need this to retain the tuples for the lifetime of the Db...
    by_subject: DbIndex<TupleNoun, Tuple>,
    by_predicate: DbIndex<String, Tuple>,
    by_object: DbIndex<TupleNoun, Tuple>
}

impl Db {
    pub fn new() -> Self {
        Db {
            by_subject: DbIndex::new(),
            by_predicate: DbIndex::new(),
            by_object: DbIndex::new(),
        }
    }

    pub fn query() -> Vec<Tuple> {
        todo!();
    }

    pub fn claim(&mut self, t: Tuple) {
        match self.by_subject.get_mut(&t.subject) {
            None => {
                let mut set = HashSet::new();
                set.insert(t.clone());
                self.by_subject.insert(t.subject.clone(), set);
            }
            Some(set) => {
                set.insert(t.clone());
            }
        }

        match self.by_predicate.get_mut(&t.predicate) {
            None => {
                let mut set = HashSet::new();
                set.insert(t.clone());
                self.by_predicate.insert(t.predicate.clone(), set);
            }
            Some(set) => {
                set.insert(t.clone());
            }
        }

        match self.by_object.get_mut(&t.object) {
            None => {
                let mut set = HashSet::new();
                set.insert(t.clone());
                self.by_object.insert(t.object.clone(), set);
            }
            Some(set) => {
                set.insert(t);
            }
        }
    }

    pub fn remoe_claim(&mut self, tuple: Tuple) {
        match self.by_subject.get_mut(&tuple.subject) {
            None => {}
            Some(set) => {
                set.remove(&tuple);
            }
        }

        match self.by_predicate.get_mut(&tuple.predicate) {
            None => {}
            Some(set) => {
                set.remove(&tuple);
            }
        }

        match self.by_object.get_mut(&tuple.object) {
            None => {}
            Some(set) => {
                set.remove(&tuple);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn db_stores_tuple() {
        let mut db = Db::new();
        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        db.claim(tuple.clone());

        let mut expected_hash = HashSet::new();
        expected_hash.insert(tuple.clone());

        assert_eq!(db.by_subject.get(&tuple.subject), Some(&expected_hash));
        assert_eq!(db.by_predicate.get(&tuple.predicate), Some(&expected_hash));
        assert_eq!(db.by_object.get(&tuple.object), Some(&expected_hash));
    }

    #[test]
    fn db_removes_tuple() {
        let mut db = Db::new();
        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        db.claim(tuple.clone());

        let mut expected_hash = HashSet::new();
        expected_hash.insert(tuple.clone());

        assert_eq!(db.by_subject.get(&tuple.subject), Some(&expected_hash));
        assert_eq!(db.by_predicate.get(&tuple.predicate), Some(&expected_hash));
        assert_eq!(db.by_object.get(&tuple.object), Some(&expected_hash));

        let empty_hash = HashSet::new();

        db.remoe_claim(tuple.clone());
        assert_eq!(db.by_subject.get(&tuple.subject), Some(&empty_hash));
        assert_eq!(db.by_predicate.get(&tuple.predicate), Some(&empty_hash));
        assert_eq!(db.by_object.get(&tuple.object), Some(&empty_hash));
    }

}