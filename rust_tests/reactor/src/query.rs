use crate::tuple::TupleNoun;
use crate::tuple::TupleNoun::Str;

pub struct Query {
    pub subject: Option<TupleNoun>,
    pub predicate: Option<String>,
    pub object: Option<TupleNoun>
}

impl Query {
    pub fn from_strs(subject: Option<&str>, predicate: Option<&str>, object: Option<&str>) -> Query {
        Query {
            subject: subject.map(|s| TupleNoun::Str(s.to_string())),
            predicate: predicate.map(|s| s.to_string()),
            object: object.map(|s| TupleNoun::Str(s.to_string())),
        }
    }
}