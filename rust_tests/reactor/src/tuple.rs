use std::ffi::c_void;
use std::ptr::NonNull;

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub enum TupleNoun {
    CPtr(NonNull<c_void>),
    Str(String)
}

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct Tuple {
    pub subject: TupleNoun,
    pub predicate: String,
    pub object: TupleNoun
}

impl Tuple {
    pub fn new_strs(subject: &str, predicate: &str, object: &str) -> Tuple {
        Self::new_strings(subject.to_string(), predicate.to_string(), object.to_string())
    }

    pub fn new_strings(subject: String, predicate: String, object: String) -> Tuple {
        Tuple {
            subject: TupleNoun::Str(subject),
            predicate: predicate,
            object: TupleNoun::Str(object),
        }
    }
}