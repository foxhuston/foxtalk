use std::fmt::Debug;
use byteorder::{ByteOrder, NativeEndian};
use crate::{read_foxtalk_size, FoxTalkDeserializable, FoxTalkSerializable, FoxtalkSize, ReturnPosition};

const QUERY_TYPE: u8 = 0;
const PREFIX_TYPE: u8 = 5;
const SYMBOL_TYPE: u8 = 1;
const CPTR_TYPE: u8 = 2;
const U64_TYPE: u8 = 3;
const I64_TYPE: u8 = 4;


#[derive(PartialEq, Clone, Eq, Hash)]
pub enum TupleNoun {
    Query,          // 0
    Prefix,         // 5 TODO: Should we renumber?
    Symbol(String), // 1
    CPtr(u64),      // 2
    U64(u64),       // 3
    I64(i64),       // 4
}

impl TupleNoun {
    pub fn from_str(s: &str) -> TupleNoun {
        match s {
            "*" => { TupleNoun::Query },
            "..." => { TupleNoun::Prefix },
            s => TupleNoun::Symbol(s.to_string()),
        }
    }
}

impl Debug for TupleNoun {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            TupleNoun::Query => {
                write!(f, "Query")
            }
            TupleNoun::Prefix => {
                write!(f, "Prefix")
            }
            TupleNoun::Symbol(s) => {
                write!(f, "Symbol({})", s)
            }
            TupleNoun::CPtr(u) => {
                write!(f, "CPtr({})", u)
            }
            TupleNoun::U64(u) => {
                write!(f, "U64({})", u)
            }
            TupleNoun::I64(i) => {
                write!(f, "I64({})", i)
            }
        }
    }
}

fn write_type_to_buffer(noun: &TupleNoun, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
    match noun {
        TupleNoun::Query => {
            write_to[start_position] = QUERY_TYPE;
        }
        TupleNoun::Prefix => {
            write_to[start_position] = PREFIX_TYPE;
        }
        TupleNoun::Symbol(_) => {
            write_to[start_position] = SYMBOL_TYPE;
        }
        TupleNoun::CPtr(_) => {
            write_to[start_position] = CPTR_TYPE;
        }
        TupleNoun::U64(_) => {
            write_to[start_position] = U64_TYPE;
        }
        TupleNoun::I64(_) => {
            write_to[start_position] = I64_TYPE;
        },
    }
    ReturnPosition { pos: start_position + size_of::<u8>() }
}


impl FoxTalkSerializable for TupleNoun {
    fn write_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        match self {
            TupleNoun::Query => {
                write_type_to_buffer(self, write_to, start_position)
            }
            TupleNoun::Prefix => {
                write_type_to_buffer(self, write_to, start_position)
            }
            TupleNoun::Symbol(value) => {
                let current_position = write_type_to_buffer(self, write_to, start_position);
                // If string len is > u32, this will probably overflow?
                let string_length = value.len() as FoxtalkSize;

                let length_bytes: [u8; size_of::<FoxtalkSize>()] = string_length.to_ne_bytes();
                let length_end_idx = (current_position.pos) + size_of::<FoxtalkSize>();
                write_to[current_position.pos..length_end_idx].copy_from_slice(&length_bytes);

                let utf8_bytes = value.as_bytes();
                let utf8_end_idx = length_end_idx + value.len();

                write_to[length_end_idx..utf8_end_idx].copy_from_slice(utf8_bytes);

                ReturnPosition { pos: utf8_end_idx }
            }
            TupleNoun::CPtr(value) => {
                let current_position = write_type_to_buffer(self, write_to, start_position);
                let u64_bytes: [u8; size_of::<u64>()] = value.to_ne_bytes();
                let e = (current_position.pos) + size_of::<u64>();
                write_to[current_position.pos..e].copy_from_slice(&u64_bytes);
                ReturnPosition { pos: e }
            }
            TupleNoun::U64(value) => {
                let current_position = write_type_to_buffer(self, write_to, start_position);
                let u64_bytes: [u8; size_of::<u64>()] = value.to_ne_bytes();
                let e = (current_position.pos) + size_of::<u64>();
                write_to[current_position.pos..e].copy_from_slice(&u64_bytes);
                ReturnPosition { pos: e }
            }
            TupleNoun::I64(value) => {
                let current_position = write_type_to_buffer(self, write_to, start_position);
                let i64_bytes: [u8; size_of::<i64>()] = value.to_ne_bytes();
                let e = (current_position.pos) + size_of::<i64>();
                write_to[current_position.pos..e].copy_from_slice(&i64_bytes);
                ReturnPosition { pos: e }
            }
        }
    }
}

impl FoxTalkDeserializable for TupleNoun {
    fn read_from_buffer(read_from: &[u8], start_position: usize) -> (Self, ReturnPosition) {
        let type_input = &read_from[start_position];
        let current_position = ReturnPosition { pos: start_position + size_of::<u8>() };

        match type_input {
            0 => {
                (TupleNoun::Query, current_position)
            }
            1 => {
                let (symbol_length, current_position) = read_foxtalk_size(read_from, current_position.pos);
                let symbol_bytes = &read_from[current_position.pos..(current_position.pos + symbol_length as usize)];
                let symbol = String::from_utf8(symbol_bytes.to_vec()).unwrap();
                (TupleNoun::Symbol(symbol), ReturnPosition { pos: current_position.pos + symbol_length as usize })
            }
            2 => {
                let bytes = &read_from[current_position.pos..(current_position.pos + size_of::<u64>())];
                (TupleNoun::CPtr(NativeEndian::read_u64(bytes)), ReturnPosition { pos: current_position.pos + size_of::<u64>() })
            }
            3 => {
                let bytes = &read_from[current_position.pos..(current_position.pos + size_of::<u64>())];
                (TupleNoun::U64(NativeEndian::read_u64(bytes)), ReturnPosition { pos: current_position.pos + size_of::<u64>() })
            }
            4 => {
                let bytes = &read_from[current_position.pos..(current_position.pos + size_of::<i64>())];
                (TupleNoun::I64(NativeEndian::read_i64(bytes)), ReturnPosition { pos: current_position.pos + size_of::<i64>() })
            }
            _ => {
                panic!("UNKNOWN TUPLENOUN TYPE CASE! Value {} at position {}", type_input, start_position)
            }
        }
    }
}
