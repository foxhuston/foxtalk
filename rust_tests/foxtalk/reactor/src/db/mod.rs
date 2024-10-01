use crate::tuple::{Tuple, TupleNoun};

use std::collections::{HashMap, HashSet};
use std::hash::Hash;

#[repr(transparent)]
pub struct DbIndex<K, V> { map: HashMap<K, HashSet<V>> }

impl<K, V> DbIndex<K, V>
    where K: PartialEq + Eq + Hash,
          V: Hash + Eq
{
    pub fn new() -> Self {
        DbIndex { map: HashMap::new() }
    }

    pub fn insert(&mut self, key: K, value: V) {
        match self.map.get_mut(&key) {
            None => {
                let mut hs = HashSet::new();
                hs.insert(value);
                self.map.insert(key, hs);
            }
            Some(mut hs) => {
                hs.insert(value);
            }
        }
    }

    pub fn contains(&self, key: &K, value: &V) -> bool {
        match self.map.get(key) {
            None => { false }
            Some(hs) => { hs.contains(value) }
        }
    }

    pub fn get(&self, key: &K) -> Option<&HashSet<V>> {
        self.map.get(key)
    }

    pub fn get_mut(&mut self, key: &K) -> Option<&mut HashSet<V>> {
        self.map.get_mut(key)
    }

    pub fn remove_all_by_key(&mut self, key: &K) -> Option<HashSet<V>> {
        self.map.remove(key)
    }

    pub fn remove_all_by_value(&mut self, value: &V) {
        self.map.iter_mut().for_each(|(_, vs)| {
            vs.remove(value);
        });
    }

    pub fn remove(&mut self, key: &K, value: &V) {
        match self.map.get_mut(key) {
            None => {}
            Some(mut hs) => { hs.remove(value); }
        }
    }

}

pub struct Db {
    by_subject: DbIndex<TupleNoun, Tuple>,
    by_predicate: DbIndex<TupleNoun, Tuple>,
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
    pub fn query(&self, query: Tuple) -> HashSet<Tuple> {
        let empty: HashSet<Tuple> = HashSet::new();

        let by_subj: Option<&HashSet<Tuple>> =
            if query.subject.is_query() { None }
            else { Some(self.by_subject.get(&query.subject).unwrap_or(&empty)) };

        let by_pred: Option<&HashSet<Tuple>> =
            if query.predicate.is_query() { None }
            else { Some(self.by_predicate.get(&query.predicate).unwrap_or(&empty)) };

        let by_obj: Option<&HashSet<Tuple>> =
            if query.object.is_query() { None }
            else { Some(self.by_object.get(&query.object).unwrap_or(&empty)) };

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
        assert_ne!(t.subject, TupleNoun::Query());
        assert_ne!(t.predicate, TupleNoun::Query());
        assert_ne!(t.object, TupleNoun::Query());

        self.by_subject.insert(t.subject.clone(), t.clone());
        self.by_predicate.insert(t.predicate.clone(), t.clone());
        self.by_object.insert(t.object.clone(), t.clone());
    }

    pub fn remove_claim(&mut self, tuple: Tuple) {
        self.by_subject.remove(&tuple.subject, &tuple);
        self.by_predicate.remove(&tuple.predicate, &tuple);
        self.by_object.remove(&tuple.object, &tuple);
    }
}

// impl Drop for Db {
//     fn drop(&mut self) {
//         let mut tuple: HashSet<Tuple> = self.by_subject.into_iter().flatten().collect();
//         for t in tuple.drain() {
//             unsafe { t.cleanup(); }
//         }
//     }
// }

///// TESTS ////////////////////////////////////////////////////////////////////

#[cfg(test)]
mod tests {
    use super::*;
    use crate::tuple::test_helpers::*;

    ///// THE BASICS ///////////////////////////////////////////////////////////
    #[test]
    fn db_stores_tuple() {
        let mut db = Db::new();
        let tuple = mk_tuple("lexi", "is a", "husky");
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
        let tuple = mk_tuple("lexi", "is a", "husky");
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
        let tuple = mk_tuple("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(mk_tuple("fox", "is a", "demon fox"));

        let results = db.query(mk_query(Some("lexi"), None, None));
        assert_eq!(results.len(), 1);
        assert!(results.contains(&tuple));

        let results = db.query(mk_query(Some("ammy"), None, None));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

    #[test]
    fn db_query_predicate() {
        let mut db = Db::new();
        let tuple = mk_tuple("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(mk_tuple("fox", "is a", "demon fox"));

        let results = db.query(mk_query(None, Some("is a"), None));
        assert_eq!(results.len(), 2);
        assert!(results.contains(&tuple));

        let results = db.query(mk_query(None, Some("is highlighted"), None));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

    #[test]
    fn db_query_object() {
        let mut db = Db::new();
        let tuple = mk_tuple("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(mk_tuple("fox", "is a", "demon fox"));

        let results = db.query(mk_query(None, None, Some("husky")));
        assert_eq!(results.len(), 1);
        assert!(results.contains(&tuple));

        let results = db.query(mk_query(None, None, Some("kitten")));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

    #[test]
    fn db_query_subject_and_predicate() {
        let mut db = Db::new();
        let tuple = mk_tuple("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(mk_tuple("fox", "is a", "demon fox"));

        let results = db.query(mk_query(Some("lexi"), Some("is a"), None));
        assert_eq!(results.len(), 1);
        assert!(results.contains(&tuple));

        let results = db.query(mk_query(Some("lexi"), Some("is highlighted"), None));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));

        let results = db.query(mk_query(Some("fox"), Some("is highlighted"), None));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

    #[test]
    fn db_query_subject_and_object() {
        let mut db = Db::new();
        let tuple = mk_tuple("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(mk_tuple("fox", "is a", "demon fox"));

        let results = db.query(mk_query(Some("lexi"), None, Some("husky")));
        assert_eq!(results.len(), 1);
        assert!(results.contains(&tuple));

        let results = db.query(mk_query(Some("lexi"), None, Some("demon fox")));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));

        let results = db.query(mk_query(Some("fox"), None, Some("husky")));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

    #[test]
    fn db_query_predicate_and_object() {
        let mut db = Db::new();
        let tuple = mk_tuple("lexi", "is a", "husky");
        db.claim(tuple.clone());
        db.claim(mk_tuple("fox", "is a", "demon fox"));

        let results = db.query(mk_query(None, Some("is a"), Some("husky")));
        assert_eq!(results.len(), 1);
        assert!(results.contains(&tuple));

        let results = db.query(mk_query(None, Some("is a"), Some("blue")));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));

        let results = db.query(mk_query(None, Some("is highlighted"), Some("husky")));
        assert_eq!(results.len(), 0);
        assert!(!results.contains(&tuple));
    }

}