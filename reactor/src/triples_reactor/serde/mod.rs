use std::collections::HashSet;
use byteorder::{ByteOrder, NativeEndian};
use crate::triples_reactor::Tuple;

#[derive(Debug, PartialEq, Clone, Eq, Hash)]
pub enum TupleNoun {
    Query,          // 0
    Symbol(String), // 1
    CPtr(u64),      // 2
    U64(u64),       // 3
    I64(i64),       // 4
}

// impl FoxTalkSerializable for Vec<Tuple> {
//
//     fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
//
//         let num_tuples = self.len() as FoxtalkSize;
//
//         let num_tuples_bytes: [u8;size_of::<FoxtalkSize>()] = num_tuples.to_ne_bytes();
//         let mut current_position = (start_position) + size_of::<FoxtalkSize>();
//         write_to[start_position..current_position].copy_from_slice(&num_tuples_bytes);
//         let mut pos_after_tuple = current_position;
//         for t in self {
//             let ret_position = t.write_to_buffer(write_to, current_position);
//             pos_after_tuple = ret_position.pos
//         }
//         ReturnPosition { pos: pos_after_tuple }
//     }
// }



impl FoxTalkSerializable for HashSet<Tuple> {
    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        let num_tuples = self.len() as FoxtalkSize;

        let num_tuples_bytes: [u8;size_of::<FoxtalkSize>()] = num_tuples.to_ne_bytes();
        let mut current_position = (start_position) + size_of::<FoxtalkSize>();
        write_to[start_position..current_position].copy_from_slice(&num_tuples_bytes);
        let mut pos_after_tuple = current_position;
        for t in self {
            let ret_position = t.write_to_buffer(write_to, current_position);
            pos_after_tuple = ret_position.pos
        }
        ReturnPosition { pos: pos_after_tuple }
    }
}
impl FoxTalkSerializable for Vec<&Tuple> {
    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        let num_tuples = self.len() as FoxtalkSize;

        let num_tuples_bytes: [u8;size_of::<FoxtalkSize>()] = num_tuples.to_ne_bytes();
        let mut current_position = (start_position) + size_of::<FoxtalkSize>();
        write_to[start_position..current_position].copy_from_slice(&num_tuples_bytes);
        let mut pos_after_tuple = current_position;
        for &t in self {
            let ret_position = t.write_to_buffer(write_to, current_position);
            pos_after_tuple = ret_position.pos
        }
        ReturnPosition { pos: pos_after_tuple }
    }
}

impl FoxTalkSerializable for Vec<Tuple> {
    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        let num_tuples = self.len() as FoxtalkSize;

        let num_tuples_bytes: [u8;size_of::<FoxtalkSize>()] = num_tuples.to_ne_bytes();
        let mut current_position = (start_position) + size_of::<FoxtalkSize>();
        write_to[start_position..current_position].copy_from_slice(&num_tuples_bytes);
        let mut pos_after_tuple = current_position;
        for t in self {
            let ret_position = t.write_to_buffer(write_to, current_position);
            pos_after_tuple = ret_position.pos
        }
        ReturnPosition { pos: pos_after_tuple }
    }
}

impl FoxTalkSerializable for Tuple {
    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        (&self).write_to_buffer(write_to, start_position)
    }
}

impl FoxTalkSerializable for &Tuple {
    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {

        let num_nouns = self.0.len() as FoxtalkSize;
        let num_nouns_bytes: [u8;size_of::<FoxtalkSize>()] = num_nouns.to_ne_bytes();
        let mut current_position = (start_position) + size_of::<FoxtalkSize>();
        write_to[start_position..current_position].copy_from_slice(&num_nouns_bytes);


        for noun in self.0.iter() {
            let ret_position = noun.write_to_buffer(write_to, current_position);
            current_position = ret_position.pos;
        }
        ReturnPosition{pos: current_position}

    }
}

impl TupleNoun {
    fn write_type_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        match self {
            TupleNoun::Query => {
                write_to[start_position] = Self::QUERY_TYPE;
            }
            TupleNoun::Symbol(_) => {
                write_to[start_position] = Self::SYMBOL_TYPE;
            }
            TupleNoun::CPtr(_) => {
                write_to[start_position] = Self::CPTR_TYPE;
            }
            TupleNoun::U64(_) => {
                write_to[start_position] = Self::U64_TYPE;
            }
            TupleNoun::I64(_) => {
                write_to[start_position] = Self::I64_TYPE;
            }
        }
        ReturnPosition{pos: start_position + size_of::<u8>()}
    }


    fn write_data_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        match self {
            TupleNoun::Query => {
                self.write_type_to_buffer(write_to, start_position)
            },
            TupleNoun::Symbol(value) => {

                let current_position = self.write_type_to_buffer(write_to, start_position);
                // If string len is > u32, this will probably overflow?
                let string_length = value.len() as FoxtalkSize;

                let length_bytes: [u8;size_of::<FoxtalkSize>()] = string_length.to_ne_bytes();
                let length_end_idx = (current_position.pos) + size_of::<FoxtalkSize>();
                write_to[current_position.pos..length_end_idx].copy_from_slice(&length_bytes);

                let utf8_bytes = value.as_bytes();
                let utf8_end_idx = length_end_idx + value.len();

                write_to[length_end_idx..utf8_end_idx].copy_from_slice(utf8_bytes);

                ReturnPosition{pos: utf8_end_idx}
            }
            TupleNoun::CPtr(value) => {
                let current_position = self.write_type_to_buffer(write_to, start_position);
                let u64_bytes: [u8;size_of::<u64>()] = value.to_ne_bytes();
                let e = (current_position.pos) + size_of::<u64>();
                write_to[current_position.pos..e].copy_from_slice(&u64_bytes);
                ReturnPosition{pos: e}
            }
            TupleNoun::U64(value) => {
                let current_position = self.write_type_to_buffer(write_to, start_position);
                let u64_bytes: [u8;size_of::<u64>()] = value.to_ne_bytes();
                let e = (current_position.pos) + size_of::<u64>();
                write_to[current_position.pos..e].copy_from_slice(&u64_bytes);
                ReturnPosition{pos: e}
            }
            TupleNoun::I64(value) => {

                let current_position = self.write_type_to_buffer(write_to, start_position);
                let i64_bytes: [u8;size_of::<i64>()] = value.to_ne_bytes();
                let e = (current_position.pos) + size_of::<i64>();
                write_to[current_position.pos..e].copy_from_slice(&i64_bytes);
                ReturnPosition{pos: e}
            }
        }
    }
}

impl FoxTalkSerializable for TupleNoun {
    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        self.write_data_to_buffer(write_to, start_position)
    }
}

#[repr(transparent)]
#[derive(PartialEq, Eq, Debug)]
pub struct ReturnPosition { pub pos: usize }

pub(crate) type FoxtalkSize = u32;

pub trait FoxTalkSerializable {

    const QUERY_TYPE: u8 = 0;
    const SYMBOL_TYPE: u8 = 1;
    const CPTR_TYPE: u8 = 2;
    const U64_TYPE: u8 = 3;
    const I64_TYPE: u8 = 4;

    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition;
}


#[inline]
fn read_foxtalk_size(input: &[u8], start_position: usize) -> (FoxtalkSize, ReturnPosition) {
    let size_bytes = &input[start_position..(start_position + size_of::<u32>())];
    (NativeEndian::read_u32(size_bytes), ReturnPosition{ pos: start_position + size_of::<FoxtalkSize>()})
}


pub(crate) fn parse_type(input: &[u8], start_position: usize) -> (TupleNoun, ReturnPosition) {
    let type_input = &input[start_position];
    let current_position = ReturnPosition{ pos: start_position + size_of::<u8>() };

    match type_input {
        0 => {
            (TupleNoun::Query, current_position)
        }
        1 => {
            let (symbol_length, current_position) = read_foxtalk_size(input, current_position.pos);
            let symbol_bytes = &input[current_position.pos..(current_position.pos + symbol_length as usize)];
            let symbol = String::from_utf8(symbol_bytes.to_vec()).unwrap();
            (TupleNoun::Symbol(symbol), ReturnPosition{ pos: current_position.pos + symbol_length as usize })
        }
        2 => {
            let bytes = &input[current_position.pos..(current_position.pos + size_of::<u64>())];
           (TupleNoun::CPtr(NativeEndian::read_u64(bytes)), ReturnPosition{ pos: current_position.pos + size_of::<u64>() })
        }
        3 => {
            let bytes = &input[current_position.pos..(current_position.pos + size_of::<u64>())];
            (TupleNoun::U64(NativeEndian::read_u64(bytes)), ReturnPosition{ pos: current_position.pos + size_of::<u64>() })
        }
        4 => {
            let bytes = &input[current_position.pos..(current_position.pos + size_of::<i64>())];
            (TupleNoun::I64(NativeEndian::read_i64(bytes)), ReturnPosition{ pos: current_position.pos + size_of::<i64>() })
        }
        _ => {
            panic!("UNKNOWN TUPLENOUN TYPE CASE! Value {} at position {}", type_input, start_position)
        }
    }

}

pub fn parse_tuples(input: &[u8]) -> Vec<Tuple> {
    let (num_tuples, mut current_position) = read_foxtalk_size(input, 0);

    let mut tuples = Vec::new();
    for _ in 0..num_tuples {
        let (num_nouns, new_position) = read_foxtalk_size(input, current_position.pos);
        current_position = new_position;
        let mut out_nouns = Vec::with_capacity(num_nouns as usize);

        for _ in 0..num_nouns {
            let (noun, new_position) = parse_type(&input, current_position.pos);
            current_position = new_position;
            out_nouns.push(noun);
        }
        tuples.push(Tuple(out_nouns));
    }
    tuples
}


#[cfg(test)]
mod tests {
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

        let tuples = parse_tuples(&bytes);

        let Tuple(nouns) = &tuples[0];

        assert_eq!(nouns.len(), 3);

        assert_eq!(nouns[0], TupleNoun::Symbol("lexi".to_string()));
        assert_eq!(nouns[1], TupleNoun::Symbol("is a".to_string()));
        assert_eq!(nouns[2], TupleNoun::Symbol("husky".to_string()));

    }

    pub fn round_trip_works_for_noun(noun: TupleNoun) {
        let buffer = &mut [0u8; 1024];

        let new_position_after_write = noun.write_to_buffer(buffer, 0);
        let (deserialized, new_position_after_read) = parse_type(buffer, 0);

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

        let _ = tuples.write_to_buffer(buffer, 0);

        let deserialized = &parse_tuples(buffer);

        assert_eq!(&tuples, deserialized)
    }
}