use crate::tuple::{Tuple, TupleNoun};
use crate::tuple::TupleNoun::Str;

#[derive(PartialEq, Eq, Debug, Clone)]
pub struct Query {
    pub subject: Option<TupleNoun>,
    pub predicate: Option<String>,
    pub object: Option<TupleNoun>
}

impl Query {
    pub fn from_strs(subject: Option<&str>, predicate: Option<&str>, object: Option<&str>) -> Self {
        Query {
            subject: subject.map(|s| TupleNoun::Str(s.to_string())),
            predicate: predicate.map(|s| s.to_string()),
            object: object.map(|s| TupleNoun::Str(s.to_string())),
        }
    }

    pub fn from_tuple(t: Tuple) -> Self {
        Query {
            subject: Some(t.subject),
            predicate: Some(t.predicate),
            object: Some(t.object)
        }
    }
}