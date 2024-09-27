use crate::tuple::{Tuple, TupleNoun};

use std::collections::{HashMap, HashSet};
use crate::query::Query;
use crate::when::{When};

pub type DbIndex<K, V> = HashMap<K, HashSet<V>>;

pub struct Db {
    by_subject: DbIndex<TupleNoun, Tuple>,
    by_predicate: DbIndex<String, Tuple>,
    by_object: DbIndex<TupleNoun, Tuple>,
}

impl Db {
    pub fn new() -> Self {
        Db {
            by_subject: DbIndex::new(),
            by_predicate: DbIndex::new(),
            by_object: DbIndex::new(),
        }
    }

    // TODO: This might should be crate-private?
    pub fn query(&self, query: Query) -> HashSet<Tuple> {
        let empty = HashSet::new();

        let by_subj: Option<&HashSet<Tuple>> =
            query.subject.map(|subj| { self.by_subject.get(&subj).unwrap_or(&empty) });


        let by_pred: Option<&HashSet<Tuple>> =
            query.predicate.map(|pred| { self.by_predicate.get(&pred).unwrap_or(&empty) });

        let by_obj: Option<&HashSet<Tuple>> =
            query.object.map(|obj| { self.by_object.get(&obj).unwrap_or(&empty) });

        // Want to return the intersection of non-None sets.

        let out: Option<HashSet<Tuple>> = vec![by_subj, by_pred, by_obj].into_iter()
            .fold(None, |acc, x| {
                match acc {
                    None => x.map(|x| x.into_iter().cloned().collect() ),
                    Some(acc_set) =>
                        match x {
                            None => Some(acc_set),
                            Some(x_set) => Some(acc_set.intersection(x_set).cloned().collect()),
                        }
                }
            });

        out.unwrap_or(HashSet::new())
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

    pub fn remove_claim(&mut self, tuple: Tuple) {
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

///// TESTS ////////////////////////////////////////////////////////////////////

#[cfg(test)]
mod tests {
    use super::*;
    use crate::when::{When};
    use crate::tuple::TupleNoun::*;
    use crate::query::Query;

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

        db.remove_claim(tuple.clone());
        assert_eq!(db.by_subject.get(&tuple.subject), Some(&empty_hash));
        assert_eq!(db.by_predicate.get(&tuple.predicate), Some(&empty_hash));
        assert_eq!(db.by_object.get(&tuple.object), Some(&empty_hash));
    }

    ///// QUERY TESTS //////////////////////////////////////////////////////////
    #[test]
    fn db_query_subject() {
        let mut db = Db::new();
        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(Tuple::new_strs("fox", "is a", "demon fox"));

        let results = db.query(Query::from_strs(Some("lexi"), None, None));
        assert_eq!(results.len(), 1);
        assert!(results.contains(&tuple));

        let results = db.query(Query::from_strs(Some("ammy"), None, None));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

    #[test]
    fn db_query_predicate() {
        let mut db = Db::new();
        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(Tuple::new_strs("fox", "is a", "demon fox"));

        let results = db.query(Query::from_strs(None, Some("is a"), None));
        assert_eq!(results.len(), 2);
        assert!(results.contains(&tuple));

        let results = db.query(Query::from_strs(None, Some("is highlighted"), None));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

    #[test]
    fn db_query_object() {
        let mut db = Db::new();
        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(Tuple::new_strs("fox", "is a", "demon fox"));

        let results = db.query(Query::from_strs(None, None, Some("husky")));
        assert_eq!(results.len(), 1);
        assert!(results.contains(&tuple));

        let results = db.query(Query::from_strs(None, None, Some("kitten")));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

    #[test]
    fn db_query_subject_and_predicate() {
        let mut db = Db::new();
        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(Tuple::new_strs("fox", "is a", "demon fox"));

        let results = db.query(Query::from_strs(Some("lexi"), Some("is a"), None));
        assert_eq!(results.len(), 1);
        assert!(results.contains(&tuple));

        let results = db.query(Query::from_strs(Some("lexi"), Some("is highlighted"), None));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));

        let results = db.query(Query::from_strs(Some("fox"), Some("is highlighted"), None));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

    #[test]
    fn db_query_subject_and_object() {
        let mut db = Db::new();
        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(Tuple::new_strs("fox", "is a", "demon fox"));

        let results = db.query(Query::from_strs(Some("lexi"), None, Some("husky")));
        assert_eq!(results.len(), 1);
        assert!(results.contains(&tuple));

        let results = db.query(Query::from_strs(Some("lexi"), None, Some("demon fox")));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));

        let results = db.query(Query::from_strs(Some("fox"), None, Some("husky")));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

    #[test]
    fn db_query_predicate_and_object() {
        let mut db = Db::new();
        let tuple = Tuple::new_strs("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(Tuple::new_strs("fox", "is a", "demon fox"));

        let results = db.query(Query::from_strs(None, Some("is a"), Some("husky")));
        assert_eq!(results.len(), 1);
        assert!(results.contains(&tuple));

        let results = db.query(Query::from_strs(None, Some("is a"), Some("blue")));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));

        let results = db.query(Query::from_strs(None, Some("is highlighted"), Some("husky")));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

}