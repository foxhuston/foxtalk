use crate::{FoxTalkDeserializable, FoxTalkOwnedSerializable, FoxTalkSerializable, ReturnPosition};
use crate::tuple_noun::TupleNoun;

#[derive(Clone, Debug, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct Tuple(pub Vec<TupleNoun>);


impl Tuple {
    // #[cfg(test)]
    pub fn triple_from_strs(s: &[&str]) -> Self {
        Tuple(
            s.iter().map(|s| TupleNoun::from_str(s)).collect()
        )
    }

    pub fn triple_from_sss(s: &str, p: &str, o: &str) -> Self {
        Tuple(vec![
            TupleNoun::Symbol(s.to_string()),
            TupleNoun::Symbol(p.to_string()),
            TupleNoun::Symbol(o.to_string())])
    }
    pub fn triple_from_ssss(s: &str, p: &str, o: &str, r: &str) -> Self {
        Tuple(vec![
            TupleNoun::Symbol(s.to_string()),
            TupleNoun::Symbol(p.to_string()),
            TupleNoun::Symbol(o.to_string()),
            TupleNoun::Symbol(r.to_string())])
    }

    // #[cfg(test)]
    pub fn triple_from_ssu(s: &str, p: &str, o: u64) -> Self {
        Tuple(vec![TupleNoun::Symbol(s.to_string()),
                   TupleNoun::Symbol(p.to_string()),
                   TupleNoun::U64(o)])
    }

    pub fn is_handler_tuple(&self) -> Option<String> {
        let Tuple(nouns) = self;
        match &nouns[..] {
            [TupleNoun::Symbol(s), TupleNoun::Symbol(p), TupleNoun::Symbol(o)] if p == "is a" && o == "handler" => { Some(s.clone()) },
            _ => None
        }
    }

}

impl FoxTalkSerializable for Tuple {
    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        let Tuple(nouns) = self;
        nouns.iter().write_to_buffer(write_to, start_position)
    }
}

impl FoxTalkDeserializable for Tuple {
    fn read_from_buffer(read_from: &[u8], start_position: usize) -> (Self, ReturnPosition) {
        let (nouns, ret) = Vec::<TupleNoun>::read_from_buffer(read_from, start_position);
        (Tuple(nouns), ret)
    }
}

