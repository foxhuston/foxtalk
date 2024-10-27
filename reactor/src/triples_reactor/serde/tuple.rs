use crate::reactor::ReactorData;
use crate::triples_reactor::serde::*;
use crate::triples_reactor::{Tuple, TupleNoun};

impl ReactorData for Tuple {}
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

