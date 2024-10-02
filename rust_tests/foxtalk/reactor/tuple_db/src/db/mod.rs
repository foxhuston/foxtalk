mod db_index;
use crate::tuple::{Tuple, TupleNoun};

use std::collections::HashSet;
use std::sync::Arc;
pub use db_index::DbIndex;


pub struct Db {
    tuples: HashSet<Arc<Tuple>>,

    by_subject: DbIndex<Arc<TupleNoun>, Arc<Tuple>>,
    by_predicate: DbIndex<Arc<TupleNoun>, Arc<Tuple>>,
    by_object: DbIndex<Arc<TupleNoun>, Arc<Tuple>>,
}

impl<'a> Db {
    pub fn new() -> Db {
        Db {
            tuples: HashSet::new(),

            by_subject: DbIndex::new(),
            by_predicate: DbIndex::new(),
            by_object: DbIndex::new(),
        }
    }

    // fn intersect_subj(&self, subject: &Arc<TupleNoun>, from: Option<&HashSet<Arc<Tuple>>>, with: &HashSet<Arc<Tuple>>) -> Option<HashSet<Arc<Tuple>>> {
    //     if !subject.is_query() {
    //         match from {
    //             None => { None }
    //             Some(by_subject) => {
    //                 Some(by_subject.intersection(with).cloned().collect())
    //             }
    //         }
    //     } else {
    //         None
    //     }
    // }



    // TODO: This might should be crate-private?
    pub fn query(&self, query: Arc<Tuple>) -> HashSet<Arc<Tuple>> {

        // This is not efficient. Let's benchmark and determine how inefficent it is.
        // We can trade write and read time off by building more indices at write-time.

        
        let results = self.tuples.iter().filter(|t| {
            let s = &t.subject;
            let p = &t.predicate;
            let o = &t.object;

            let subject_matches = query.subject.is_query() || &query.subject == s;
            let predicate_matches = query.predicate.is_query() || &query.predicate == p;
            let object_matches = query.object.is_query() || &query.object == o;

            subject_matches && predicate_matches && object_matches
        }).cloned().collect::<HashSet<Arc<Tuple>>>();

        results
        //
        // let w_subj = self.intersect_subj(&query.subject, self.by_subject.get(&query.subject), &self.tuples);
        // let w_pred =
        //     w_subj.and_then(|s| self.intersect_subj(&query.predicate, self.by_predicate.get(&query.predicate), &s))
        //         .or_else(|| self.intersect_subj(&query.predicate, self.by_predicate.get(&query.predicate), &self.tuples));
        //
        // w_pred.unwrap_or(&empty).clone()

        // let w_obj =
        //     w_pred.and_then(|s| self.intersect_subj(&query.object, self.by_object.get(&query.object), &s))
        //         .or_else(|| self.intersect_subj(&query.object, self.by_object.get(&query.object), &self.tuples));
        //
        // // TODO: should be every tuple, but that seems very expensive...
        // w_obj.unwrap_or(HashSet::new())
    }

    pub fn claim(&'a mut self, t: Arc<Tuple>) {
        assert_ne!(t.subject, Arc::new(TupleNoun::Query));
        assert_ne!(t.predicate, Arc::new(TupleNoun::Query));
        assert_ne!(t.object, Arc::new(TupleNoun::Query));

        self.tuples.insert(t.clone());

        self.by_subject.insert(t.subject.clone(), t.clone());
        self.by_predicate.insert(t.predicate.clone(), t.clone());
        self.by_object.insert(t.object.clone(), t.clone());
    }

    pub fn remove_claim(&mut self, tuple: Arc<Tuple>) {
        self.tuples.remove(&tuple);

        self.by_subject.remove(&tuple.subject, &tuple);
        self.by_predicate.remove(&tuple.predicate, &tuple);
        self.by_object.remove(&tuple.object, &tuple);
    }
}

///// TESTS ////////////////////////////////////////////////////////////////////

#[cfg(test)]
mod tests {
    use super::*;
    use crate::tuple::test_helpers::*;

    ///// THE BASICS ///////////////////////////////////////////////////////////
    #[test]
    fn db_stores_tuple() {
        let mut db = Db::new();
        db.claim(mk_tuple("lexi", "is a", "husky"));

        let expected_tuple = mk_tuple("lexi", "is a", "husky");

        let mut expected_hash = HashSet::new();
        expected_hash.insert(expected_tuple.clone());

        assert_eq!(db.by_subject.get(&expected_tuple.subject), Some(&expected_hash));
        assert_eq!(db.by_predicate.get(&expected_tuple.predicate), Some(&expected_hash));
        assert_eq!(db.by_object.get(&expected_tuple.object), Some(&expected_hash));
    }

    #[test]
    fn db_removes_tuple() {
        fn init_db() -> Db {
            let mut db = Db::new();
            db.claim(mk_tuple("lexi", "is a", "husky"));

            db
        }

        let mut db = init_db();

        let tuple = mk_tuple("lexi", "is a", "husky");

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

    // #[test]
    // fn db_query_predicate() {
    //     let mut db = Db::new();
    //     let tuple = mk_tuple("lexi", "is a", "husky");
    //     db.claim(tuple.clone());
    //     db.claim(mk_tuple("fox", "is a", "demon fox"));
    //
    //     let results = db.query(mk_query(None, Some("is a"), None));
    //     assert_eq!(results.len(), 2);
    //     assert!(results.contains(&tuple));
    //
    //     let results = db.query(mk_query(None, Some("is highlighted"), None));
    //     assert_eq!(results.len(), 0);
    //     assert!(!results.contains(&tuple));
    // }
    //
    // #[test]
    // fn db_query_object() {
    //     let mut db = Db::new();
    //     let tuple = mk_tuple("lexi", "is a", "husky");
    //     db.claim(tuple.clone());
    //     db.claim(mk_tuple("fox", "is a", "demon fox"));
    //
    //     let results = db.query(mk_query(None, None, Some("husky")));
    //     assert_eq!(results.len(), 1);
    //     assert!(results.contains(&tuple));
    //
    //     let results = db.query(mk_query(None, None, Some("kitten")));
    //     assert_eq!(results.len(), 0);
    //     assert!(!results.contains(&tuple));
    // }
    //
    // #[test]
    // fn db_query_subject_and_predicate() {
    //     let mut db = Db::new();
    //     let tuple = mk_tuple("lexi", "is a", "husky");
    //     db.claim(tuple.clone());
    //     db.claim(mk_tuple("fox", "is a", "demon fox"));
    //
    //     let results = db.query(mk_query(Some("lexi"), Some("is a"), None));
    //     assert_eq!(results.len(), 1);
    //     assert!(results.contains(&tuple));
    //
    //     let results = db.query(mk_query(Some("lexi"), Some("is highlighted"), None));
    //     assert_eq!(results.len(), 0);
    //     assert!(!results.contains(&tuple));
    //
    //     let results = db.query(mk_query(Some("fox"), Some("is highlighted"), None));
    //     assert_eq!(results.len(), 0);
    //     assert!(!results.contains(&tuple));
    // }
    //
    // #[test]
    // fn db_query_subject_and_object() {
    //     let mut db = Db::new();
    //     let tuple = mk_tuple("lexi", "is a", "husky");
    //     db.claim(tuple.clone());
    //     db.claim(mk_tuple("fox", "is a", "demon fox"));
    //
    //     let results = db.query(mk_query(Some("lexi"), None, Some("husky")));
    //     assert_eq!(results.len(), 1);
    //     assert!(results.contains(&tuple));
    //
    //     let results = db.query(mk_query(Some("lexi"), None, Some("demon fox")));
    //     assert_eq!(results.len(), 0);
    //     assert!(!results.contains(&tuple));
    //
    //     let results = db.query(mk_query(Some("fox"), None, Some("husky")));
    //     assert_eq!(results.len(), 0);
    //     assert!(!results.contains(&tuple));
    // }
    //
    // #[test]
    // fn db_query_predicate_and_object() {
    //     let mut db = Db::new();
    //     let tuple = mk_tuple("lexi", "is a", "husky");
    //     db.claim(tuple.clone());
    //     db.claim(mk_tuple("fox", "is a", "demon fox"));
    //
    //     let results = db.query(mk_query(None, Some("is a"), Some("husky")));
    //     assert_eq!(results.len(), 1);
    //     assert!(results.contains(&tuple));
    //
    //     let results = db.query(mk_query(None, Some("is a"), Some("blue")));
    //     assert_eq!(results.len(), 0);
    //     assert!(!results.contains(&tuple));
    //
    //     let results = db.query(mk_query(None, Some("is highlighted"), Some("husky")));
    //     assert_eq!(results.len(), 0);
    //     assert!(!results.contains(&tuple));
    // }

}