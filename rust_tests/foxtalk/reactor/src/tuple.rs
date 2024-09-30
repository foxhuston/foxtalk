use std::ffi::c_void;
use std::ptr::NonNull;
use crate::ffi2::c_heap_object::CHeapObject;

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub enum TupleNoun {
    Query(),
    CPtrWithFree(CHeapObject),
    Symbol(String),
    U64(u64),
    I64(i64)
}

impl TupleNoun {
    pub fn is_query(&self) -> bool {
        match self {
            TupleNoun::Query() => true,
            _ => false
        }
    }

    pub fn mk_panic_msg(&self, should_be: &'static str) -> String {
        match self {
            TupleNoun::Query() => { format!("Expected TupleNoun to be {should_be}, but it was actually a Query!") }
            TupleNoun::CPtrWithFree(_) => { format!("Expected TupleNoun to be {should_be}, but it was actually a CPtrWithFree!") }
            TupleNoun::Symbol(_) => { format!("Expected TupleNoun to be {should_be}, but it was actually a Symbol!") }
            TupleNoun::U64(_) => { format!("Expected TupleNoun to be {should_be}, but it was actually a U64!") }
            TupleNoun::I64(_) => { format!("Expected TupleNoun to be {should_be}, but it was actually an I64!") }
        }
    }
}

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
#[repr(C)]
pub struct Tuple {
    pub subject: TupleNoun,
    pub predicate: TupleNoun,
    pub object: TupleNoun
}

// #[cfg(test)]
pub mod test_helpers {
    use super::{Tuple, TupleNoun};

    pub fn mk_query(s: Option<&'static str>, p: Option<&'static str>, o: Option<&'static str>) -> Tuple {
        Tuple {
            subject: s.map(|s| { TupleNoun::Symbol(s.to_string()) }).unwrap_or(TupleNoun::Query()),
            predicate: p.map(|s| { TupleNoun::Symbol(s.to_string()) }).unwrap_or(TupleNoun::Query()),
            object: o.map(|s| { TupleNoun::Symbol(s.to_string()) }).unwrap_or(TupleNoun::Query()),
        }
    }

    pub fn mk_tuple(s: &'static str, p: &'static str, o: &'static str) -> Tuple {
        Tuple {
            subject: TupleNoun::Symbol(s.to_string()),
            predicate: TupleNoun::Symbol(p.to_string()),
            object: TupleNoun::Symbol(o.to_string()),
        }
    }
}