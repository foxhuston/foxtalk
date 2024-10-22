use crate::serde::TupleNoun;

#[derive(Clone, Debug, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct Tuple(Vec<TupleNoun>);

pub fn tuple_matches<'a>(Tuple(query): &'a Tuple) -> Box<dyn Fn(&Tuple) -> bool + 'a> {
    Box::new(|Tuple(maybe_matching_noun): &Tuple| {
        if query.len() != maybe_matching_noun.len() {
            return false;
        }
        for (i, query_noun) in query.iter().enumerate() {
            if query_noun != &maybe_matching_noun[i] && query_noun != &TupleNoun::Query {
                return false;
            }
        }
        true
    })
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn tuple_matches_works() {
        let t1 = Tuple(vec![TupleNoun::Query, TupleNoun::Symbol("is a".to_string()), TupleNoun::Symbol("husky".to_string())]);
        let t2 = Tuple(vec![TupleNoun::Symbol("lexi".to_string()), TupleNoun::Symbol("is a".to_string()), TupleNoun::Symbol("husky".to_string())]);
        let t3 = Tuple(vec![TupleNoun::Symbol("lexi".to_string()), TupleNoun::Symbol("is a".to_string()), TupleNoun::Symbol("husky".to_string()), TupleNoun::Symbol("and".to_string()), TupleNoun::Symbol("cute".to_string())]);

        let matcher = tuple_matches(&t1);
        assert!(matcher(&t2));
        assert!(!matcher(&t3));
    }
}