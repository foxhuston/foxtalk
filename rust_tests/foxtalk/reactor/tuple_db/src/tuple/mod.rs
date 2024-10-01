pub mod tuple_noun;

use std::sync::Arc;
pub use tuple_noun::TupleNoun;

#[derive(PartialEq, Eq, Hash, Debug)]
#[repr(C)]
pub struct Tuple {
    pub subject: Arc<TupleNoun>,
    pub predicate: Arc<TupleNoun>,
    pub object: Arc<TupleNoun>
}

impl Drop for Tuple {
    fn drop(&mut self) {
        println!("Dropping Tuple: {self:?}");
    }
}

// #[cfg(test)]
pub mod test_helpers {
    use std::sync::Arc;
    use ustr::ustr;
    use super::{Tuple, TupleNoun};

    pub fn mk_query(s: Option<&'static str>, p: Option<&'static str>, o: Option<&'static str>) -> Arc<Tuple> {
        Arc::new(Tuple {
            subject: Arc::new(s.map(|s| { TupleNoun::Symbol(ustr(s)) }).unwrap_or(TupleNoun::Query)),
            predicate: Arc::new(p.map(|s| { TupleNoun::Symbol(ustr(s)) }).unwrap_or(TupleNoun::Query)),
            object: Arc::new(o.map(|s| { TupleNoun::Symbol(ustr(s)) }).unwrap_or(TupleNoun::Query)),
        })
    }

    pub fn mk_tuple(s: &'static str, p: &'static str, o: &'static str) -> Arc<Tuple> {
        Arc::new(Tuple {
            subject: Arc::new(TupleNoun::Symbol(ustr(s))),
            predicate: Arc::new(TupleNoun::Symbol(ustr(p))),
            object: Arc::new(TupleNoun::Symbol(ustr(o))),
        })
    }
}