use byteorder::{ByteOrder, NativeEndian};
use crate::serde::FoxTalkType::Query;

trait FoxTalkSerializable {

    const UPSERT_OP: u8 = 0;
    const DELETE_OP: u8 = 1;
    const SELECT_OP: u8 = 2;
    const STRING_TYPE: u8 = 0;

    const INT_TYPE: u8 = 1;
    const DOUBLE_TYPE: u8 = 2;
    const BYTES_TYPE: u8 = 3;
    const FOX_TALK_ID_TYPE: u8 = 4;
    const QUERY_TYPE: u8 = 5;
    fn fox_talk_bytes(&self) -> Vec<u8>;
}

#[derive(Debug, PartialEq, Clone)]
pub enum FoxTalkOperation {
    Upsert,
    Delete,
    Select,
}


#[derive(Debug, PartialEq, Clone)]
pub enum FoxTalkType {
    Query,          // 0
    Symbol(String),  // 1
    CPtr(u64),    // 2
    U64(u64),       // 3
    I64(i64),       // 4
    // Float(f32),
    // String(String),
    // Int(u32),
    // Bytes(Vec<u8>),
}

pub fn parse_op(input: &u8) -> FoxTalkOperation {
    match input {
        0 => FoxTalkOperation::Upsert,
        1 => FoxTalkOperation::Delete,
        2 => FoxTalkOperation::Select,
        _ => panic!("Unknown opcode")
    }
}

pub fn parse_ut8_string(input: &[u8], length: usize) -> String {
    let string_data = &input[5..(length+5)];
    String::from_utf8(string_data.to_vec()).unwrap()
}

pub fn parse_bytes(input: &[u8], length: usize) -> Vec<u8> {
    input[5..(length+5)].to_vec()
}


type FoxtalkSize = u32;

#[inline]
fn read_foxtalk_size(input: &[u8], start_position: usize) -> (FoxtalkSize, usize) {
    let size_bytes = &input[start_position..(start_position + size_of::<u32>())];
    (NativeEndian::read_u32(size_bytes), size_of::<FoxtalkSize>())
}


pub fn parse_type(input: &[u8], start_position: usize) -> (FoxTalkType, usize) {
    let type_input = &input[start_position];
    let current_position = start_position + size_of::<u8>();

    match type_input {
        0 => {
            (Query, start_position + size_of::<u8>())
        }
        1 => {
            let (symbol_length, read_bytes) = read_foxtalk_size(input, current_position);
            let current_position = current_position + read_bytes;
            let symbol_bytes = &input[current_position..(current_position + symbol_length as usize)];
            let symbol = String::from_utf8(symbol_bytes.to_vec()).unwrap();
            (FoxTalkType::Symbol(symbol), current_position + symbol_length as usize)
        }
        2 => {
            let bytes = &input[current_position..(current_position + size_of::<u64>())];
            (FoxTalkType::CPtr(NativeEndian::read_u64(bytes)), size_of::<u64>())
        }
        3 => {
            let bytes = &input[current_position..(current_position + size_of::<u64>())];
            (FoxTalkType::U64(NativeEndian::read_u64(bytes)), size_of::<u64>())
        }
        4 => {
            let bytes = &input[current_position..(current_position + size_of::<i64>())];
            (FoxTalkType::I64(NativeEndian::read_i64(bytes)), size_of::<i64>())
        }
        _ => {
            panic!("UNKNOWN TUPLENOUN TYPE CASE! Value {} at position {}", type_input, start_position)
        }
    }

}

pub fn parse_row(input: &[u8]) -> (FoxTalkType, FoxTalkType, FoxTalkType) {
    let (total_byte_size, current_position) = read_foxtalk_size(input, 0);

    let (subject, current_position) = parse_type(&input, current_position);
    let (predicate, current_position) = parse_type(&input, current_position);
    let (object, current_position) = parse_type(&input, current_position);

    debug_assert_eq!(current_position, total_byte_size as usize);

    (subject, predicate, object)
}


#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    pub fn parsing_should_work() {
        // let [insert] = 0u8.to_ne_bytes();
        //
        // fn string_bytes(string: String) -> Vec<u8> {
        //     let [string_type] = 3u8.to_ne_bytes();
        //     let len = string.len() as u32;
        //     let len_bytes: [u8; 4] = len.to_ne_bytes();
        //     let string_bytes = string.as_bytes();
        //     vec!(vec!(string_type), len_bytes.to_vec(), string_bytes.to_vec()).concat()
        // }
        //
        // let bytes = [
        //     vec!(insert),
        //     string_bytes("Lexi".to_string()),
        //     string_bytes("is a".to_string()),
        //     string_bytes("husky".to_string()),
        // ].concat();
        //
        // let (parsed_op, parsed_subj, parsed_pred, parsed_obj) = parse_row(&bytes);
        // assert_eq!(parsed_op, FoxTalkOperation::Upsert);
        // assert_eq!(parsed_subj, FoxTalkType::Sy("Lexi".to_string()));
        // assert_eq!(parsed_pred, FoxTalkType::String("is a".to_string()));
        // assert_eq!(parsed_obj, FoxTalkType::String("husky".to_string()));
    }

    #[test]
    pub fn bytes_from_cpp_work() {
       let bytes = "
       20 00 00 00 01 04 00 00 00 6c 65 78 69 01 04 00
       00 00 69 73 20 61 01 05 00 00 00 68 75 73 6b 79
       00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
       00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00";

        let bytes = bytes
            .split_whitespace()
            .map(|x| x.trim())
            .map(|x| u8::from_str_radix(x, 16).unwrap())
            .collect::<Vec<u8>>();

        println!("{:?}", bytes);

        let (parsed_subj, parsed_pred, parsed_obj) = parse_row(&bytes);

        assert_eq!(parsed_subj, FoxTalkType::Symbol("lexi".to_string()));
        assert_eq!(parsed_pred, FoxTalkType::Symbol("is a".to_string()));
        assert_eq!(parsed_obj, FoxTalkType::Symbol("husky".to_string()));

    }

    #[test]
    pub fn all_types_work() {
        // let [insert] = 0u8.to_ne_bytes();
        // let [select] = 2u8.to_ne_bytes();
        //
        // fn string_bytes(string: String) -> Vec<u8> {
        //     let [string_type] = 3u8.to_ne_bytes();
        //     let len = string.len() as u32;
        //     let len_bytes: [u8; 4] = len.to_ne_bytes();
        //     let string_bytes = string.as_bytes();
        //     vec!(vec!(string_type), len_bytes.to_vec(), string_bytes.to_vec()).concat()
        // }
        //
        // fn int_bytes(int: u32) -> Vec<u8> {
        //     let [int_type] = 4u8.to_ne_bytes();
        //     let int_bytes = int.to_ne_bytes();
        //     vec!(vec!(int_type), int_bytes.to_vec()).concat()
        // }
        //
        // fn float_bytes(float: f32) -> Vec<u8> {
        //     let [float_type] = 2u8.to_ne_bytes();
        //     let float_bytes = float.to_ne_bytes();
        //     vec!(vec!(float_type), float_bytes.to_vec()).concat()
        // }
        //
        // fn bytes_bytes(bytes: Vec<u8>) -> Vec<u8> {
        //     let [bytes_type] = 5u8.to_ne_bytes();
        //     let len = bytes.len() as u32;
        //     let len_bytes: [u8; 4] = len.to_ne_bytes();
        //     vec!(vec!(bytes_type), len_bytes.to_vec(), bytes).concat()
        // }
        //
        // let bytes = [
        //     vec!(select),
        //     int_bytes(1),
        //     float_bytes(1.0),
        //     bytes_bytes(vec!(1, 2, 3, 4)),
        // ].concat();
        //
        // let (parsed_op, parsed_subj, parsed_pred, parsed_obj) = parse_row(&bytes);
        // assert_eq!(parsed_op, FoxTalkOperation::Select);
        // assert_eq!(parsed_subj, FoxTalkType::Int(1));
        // assert_eq!(parsed_pred, FoxTalkType::Float(1.0));
        // assert_eq!(parsed_obj, FoxTalkType::Bytes(vec!(1, 2, 3, 4)));

    }
}