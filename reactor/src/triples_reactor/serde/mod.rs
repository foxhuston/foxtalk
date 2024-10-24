pub mod tuple;
mod tuple_noun;

use std::collections::HashSet;
use std::hash::Hash;
use byteorder::{ByteOrder, NativeEndian};
use crate::triples_reactor::Tuple;

pub use tuple_noun::*;

#[repr(transparent)]
#[derive(PartialEq, Eq, Debug)]
pub struct ReturnPosition {
    pub pos: usize,
}

pub(crate) type FoxtalkSize = u32;

pub trait FoxTalkSerializable {
    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition;
}

pub trait FoxTalkOwnedSerializable {
    fn write_to_buffer(self, write_to: &mut [u8], start_position: usize) -> ReturnPosition;
}

pub trait FoxTalkDeserializable {
    fn read_from_buffer(read_from: &[u8], start_position: usize) -> (Self, ReturnPosition)
    where
        Self: Sized;
}


#[inline]
fn read_foxtalk_size(input: &[u8], start_position: usize) -> (FoxtalkSize, ReturnPosition) {
    let size_bytes = &input[start_position..(start_position + size_of::<u32>())];
    (NativeEndian::read_u32(size_bytes), ReturnPosition { pos: start_position + size_of::<FoxtalkSize>() })
}

impl<'a, I, T> FoxTalkOwnedSerializable for I
where
    I: Iterator<Item = &'a T>,
    T: FoxTalkSerializable + 'a,
{
    fn write_to_buffer(self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        let mut count_tuples: FoxtalkSize = 0;

        let mut current_position = (start_position) + size_of::<FoxtalkSize>();
        for t in self {
            count_tuples += 1;
            
            let ret_position = t.write_to_buffer(write_to, current_position);
            current_position = ret_position.pos
        }
        
        let num_tuples_bytes: [u8; size_of::<FoxtalkSize>()] = count_tuples.to_ne_bytes();
        write_to[start_position.. start_position + size_of::<FoxtalkSize>()].copy_from_slice(&num_tuples_bytes);
        
        ReturnPosition { pos: current_position }
    }
}

impl<T> FoxTalkDeserializable for Vec<T>
where
    T: FoxTalkDeserializable,
{
    fn read_from_buffer(read_from: &[u8], start_position: usize) -> (Self, ReturnPosition) {
        let (num_ts, mut current_position) = read_foxtalk_size(read_from, start_position);
        let mut out_ts = Vec::with_capacity(num_ts as usize);

        for _ in 0..num_ts {
            let (o, next_pos) = T::read_from_buffer(read_from, current_position.pos);
            out_ts.push(o);
            current_position = next_pos;
        }

        (out_ts, current_position)
    }
}

impl<T> FoxTalkSerializable for HashSet<T>
where
    T: FoxTalkSerializable,
{
    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        let num_tuples = self.len() as FoxtalkSize;

        let num_tuples_bytes: [u8; size_of::<FoxtalkSize>()] = num_tuples.to_ne_bytes();
        let mut current_position = (start_position) + size_of::<FoxtalkSize>();
        write_to[start_position..current_position].copy_from_slice(&num_tuples_bytes);
        let mut pos_after_tuple = current_position;
        for t in self.iter() {
            let ret_position = t.write_to_buffer(write_to, current_position);
            pos_after_tuple = ret_position.pos
        }
        ReturnPosition { pos: pos_after_tuple }
    }
}

impl<T> FoxTalkDeserializable for HashSet<T>
where
    T: FoxTalkDeserializable + Eq + Hash,
{
    fn read_from_buffer(read_from: &[u8], start_position: usize) -> (Self, ReturnPosition) {
        let (num_ts, mut current_position) = read_foxtalk_size(read_from, start_position);
        let mut out_ts = HashSet::with_capacity(num_ts as usize);

        for _ in 0..num_ts {
            let (o, next_pos) = T::read_from_buffer(read_from, current_position.pos);
            out_ts.insert(o);
            current_position = next_pos;
        }

        (out_ts, current_position)
    }
}


#[cfg(test)]
mod tests {
    use crate::triples_reactor::TupleNoun;
    use super::*;

    #[test]
    pub fn bytes_from_cpp_work() {
        let bytes = "
       01 00 00 00 03 00 00 00 01 04 00 00 00 6c 65 78 69 01 04 00
       00 00 69 73 20 61 01 05 00 00 00 68 75 73 6b 79
       00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
       00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";

        let bytes = bytes
            .split_whitespace()
            .map(|x| x.trim())
            .map(|x| u8::from_str_radix(x, 16).unwrap())
            .collect::<Vec<u8>>();

        println!("{:?}", bytes);

        let (tuples, _) = Vec::<Tuple>::read_from_buffer(&bytes, 0);

        let Tuple(nouns) = &tuples[0];

        assert_eq!(nouns.len(), 3);

        assert_eq!(nouns[0], TupleNoun::Symbol("lexi".to_string()));
        assert_eq!(nouns[1], TupleNoun::Symbol("is a".to_string()));
        assert_eq!(nouns[2], TupleNoun::Symbol("husky".to_string()));
    }

    pub fn round_trip_works_for_noun(noun: TupleNoun) {
        let buffer = &mut [0u8; 1024];

        let new_position_after_write = noun.write_to_buffer(buffer, 0);
        let (deserialized, new_position_after_read) = TupleNoun::read_from_buffer(buffer, 0);

        assert_eq!(new_position_after_write.pos, new_position_after_read.pos);
        assert_eq!(noun, deserialized)
    }

    #[test]
    pub fn round_trip_works_for_query() {
        round_trip_works_for_noun(TupleNoun::Query)
    }

    #[test]
    pub fn round_trip_works_for_symbol() {
        round_trip_works_for_noun(TupleNoun::Symbol("testing".to_string()))
    }

    #[test]
    pub fn round_trip_works_for_cptr() {
        round_trip_works_for_noun(TupleNoun::CPtr(0x834237423))
    }

    #[test]
    pub fn round_trip_works_for_u64() {
        round_trip_works_for_noun(TupleNoun::U64(1834u64))
    }

    #[test]
    pub fn round_trip_works_for_i64() {
        round_trip_works_for_noun(TupleNoun::I64(-84539))
    }

    #[test]
    pub fn round_trip_works() {
        let subj = TupleNoun::Symbol("/dev/cam1".to_string());
        let pred = TupleNoun::Symbol("is at".to_string());
        let obj = TupleNoun::CPtr(0x12345678);
        let buffer = &mut [0u8; 1024];

        let tuples = vec![Tuple(vec![subj, pred, obj])];

        let _ = tuples.iter().write_to_buffer(buffer, 0);

        let (deserialized, _) = Vec::<Tuple>::read_from_buffer(buffer, 0);

        assert_eq!(tuples, deserialized)
    }
}