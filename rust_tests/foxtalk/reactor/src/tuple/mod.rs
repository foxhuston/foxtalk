pub mod tuple_noun;
pub use tuple_noun::TupleNoun;

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
#[repr(C)]
pub struct Tuple {
    pub subject: TupleNoun,
    pub predicate: TupleNoun,
    pub object: TupleNoun
}

// #[cfg(test)]
pub mod test_helpers {
    use ustr::ustr;
    use super::{Tuple, TupleNoun};

    pub fn mk_query(s: Option<&'static str>, p: Option<&'static str>, o: Option<&'static str>) -> Tuple {
        Tuple {
            subject: s.map(|s| { TupleNoun::Symbol(ustr(s)) }).unwrap_or(TupleNoun::Query()),
            predicate: p.map(|s| { TupleNoun::Symbol(ustr(s)) }).unwrap_or(TupleNoun::Query()),
            object: o.map(|s| { TupleNoun::Symbol(ustr(s)) }).unwrap_or(TupleNoun::Query()),
        }
    }

    pub fn mk_tuple(s: &'static str, p: &'static str, o: &'static str) -> Tuple {
        Tuple {
            subject: TupleNoun::Symbol(ustr(s)),
            predicate: TupleNoun::Symbol(ustr(p)),
            object: TupleNoun::Symbol(ustr(o)),
        }
    }
}