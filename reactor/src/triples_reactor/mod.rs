use crate::triples_reactor::TupleNoun::U64;

pub mod serde;
pub mod ffi;
mod dynamic_loading_triples_reactor;

#[derive(Clone, Debug, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct Tuple(pub Vec<TupleNoun>);

impl Tuple {
    // #[cfg(test)]
    pub fn triple_from_strs(s: &str, p: &str, o: &str) -> Self {
        Tuple(vec![
            TupleNoun::Symbol(s.to_string()),
            TupleNoun::Symbol(p.to_string()),
            TupleNoun::Symbol(o.to_string())])
    }

    // #[cfg(test)]
    pub fn triple_from_ssu(s: &str, p: &str, o: u64) -> Self {
        Tuple(vec![TupleNoun::Symbol(s.to_string()),
                   TupleNoun::Symbol(p.to_string()),
                   TupleNoun::U64(o)])
    }
}

#[derive(Debug, PartialEq, Clone, Eq, Hash)]
pub enum TupleNoun {
    Query,          // 0
    Symbol(String), // 1
    CPtr(u64),      // 2
    U64(u64),       // 3
    I64(i64),       // 4
}

#[cfg(test)]
mod tests {}
