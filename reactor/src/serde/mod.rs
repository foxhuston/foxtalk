use byteorder::{ByteOrder, NativeEndian};


#[derive(Debug, PartialEq, Clone)]
pub enum FoxTalkType {
    Query,          // 0
    Symbol(String), // 1
    CPtr(u64),      // 2
    U64(u64),       // 3
    I64(i64),       // 4
}

impl FoxTalkSerializable for FoxTalkType {
    fn write_type_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        match self {
            FoxTalkType::Query => {
                write_to[start_position] = Self::QUERY_TYPE;
            }
            FoxTalkType::Symbol(_) => {
                write_to[start_position] = Self::SYMBOL_TYPE;
            }
            FoxTalkType::CPtr(_) => {
                write_to[start_position] = Self::CPTR_TYPE;
            }
            FoxTalkType::U64(_) => {
                write_to[start_position] = Self::U64_TYPE;
            }
            FoxTalkType::I64(_) => {
                write_to[start_position] = Self::I64_TYPE;
            }
        }
        ReturnPosition{pos: start_position + size_of::<u8>()}
    }


    fn write_data_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition {
        match self {
            FoxTalkType::Query => {
                self.write_type_to_buffer(write_to, start_position)
            },
            FoxTalkType::Symbol(value) => {

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
            FoxTalkType::CPtr(value) => {
                let current_position = self.write_type_to_buffer(write_to, start_position);
                let u64_bytes: [u8;size_of::<u64>()] = value.to_ne_bytes();
                let e = (current_position.pos) + size_of::<u64>();
                write_to[current_position.pos..e].copy_from_slice(&u64_bytes);
                ReturnPosition{pos: e}
            }
            FoxTalkType::U64(value) => {
                let current_position = self.write_type_to_buffer(write_to, start_position);
                let u64_bytes: [u8;size_of::<u64>()] = value.to_ne_bytes();
                let e = (current_position.pos) + size_of::<u64>();
                write_to[current_position.pos..e].copy_from_slice(&u64_bytes);
                ReturnPosition{pos: e}
            }
            FoxTalkType::I64(value) => {

                let i64_bytes: [u8;size_of::<i64>()] = value.to_ne_bytes();
                write_to[start_position] = Self::I64_TYPE;
                let s = start_position + 1;
                let e = s + size_of::<i64>();
                write_to[s..e].copy_from_slice(&i64_bytes);
                ReturnPosition{ pos: e }
            }
        }
    }
}
#[repr(transparent)]
pub(crate) struct ReturnPosition { pub pos: usize }

pub(crate) type FoxtalkSize = u32;

pub(crate) trait FoxTalkSerializable {

    const QUERY_TYPE: u8 = 0;
    const SYMBOL_TYPE: u8 = 1;
    const CPTR_TYPE: u8 = 2;
    const U64_TYPE: u8 = 3;
    const I64_TYPE: u8 = 4;

    fn write_type_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition;
    fn write_data_to_buffer(&self, write_to: &mut [u8], start_position: usize) -> ReturnPosition;
}


#[inline]
fn read_foxtalk_size(input: &[u8], start_position: usize) -> (FoxtalkSize, ReturnPosition) {
    let size_bytes = &input[start_position..(start_position + size_of::<u32>())];
    (NativeEndian::read_u32(size_bytes), ReturnPosition{ pos: start_position + size_of::<FoxtalkSize>()})
}


pub(crate) fn parse_type(input: &[u8], start_position: usize) -> (FoxTalkType, ReturnPosition) {
    let type_input = &input[start_position];
    let current_position = ReturnPosition{ pos: start_position + size_of::<u8>() };

    match type_input {
        0 => {
            (FoxTalkType::Query, current_position)
        }
        1 => {
            let (symbol_length, current_position) = read_foxtalk_size(input, current_position.pos);
            let symbol_bytes = &input[current_position.pos..(current_position.pos + symbol_length as usize)];
            let symbol = String::from_utf8(symbol_bytes.to_vec()).unwrap();
            (FoxTalkType::Symbol(symbol), ReturnPosition{ pos: current_position.pos + symbol_length as usize })
        }
        2 => {
            let bytes = &input[current_position.pos..(current_position.pos + size_of::<u64>())];
           (FoxTalkType::CPtr(NativeEndian::read_u64(bytes)), ReturnPosition{ pos: current_position.pos + size_of::<u64>() })
        }
        3 => {
            let bytes = &input[current_position.pos..(current_position.pos + size_of::<u64>())];
            (FoxTalkType::U64(NativeEndian::read_u64(bytes)), ReturnPosition{ pos: current_position.pos + size_of::<u64>() })
        }
        4 => {
            let bytes = &input[current_position.pos..(current_position.pos + size_of::<i64>())];
            (FoxTalkType::I64(NativeEndian::read_i64(bytes)), ReturnPosition{ pos: current_position.pos + size_of::<i64>() })
        }
        _ => {
            panic!("UNKNOWN TUPLENOUN TYPE CASE! Value {} at position {}", type_input, start_position)
        }
    }

}

pub fn parse_row(input: &[u8]) -> (FoxTalkType, FoxTalkType, FoxTalkType) {
    let (total_byte_size, current_position) = read_foxtalk_size(input, 0);

    let (subject, current_position) = parse_type(&input, current_position.pos);
    let (predicate, current_position) = parse_type(&input, current_position.pos);
    let (object, current_position) = parse_type(&input, current_position.pos);

    debug_assert_eq!(current_position.pos, total_byte_size as usize);

    (subject, predicate, object)
}


#[cfg(test)]
mod tests {
    use super::*;

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
    pub fn round_trip_works() {
        let subj = FoxTalkType::Symbol("/dev/cam1".to_string());
        let pred = FoxTalkType::Symbol("is at".to_string());
        let obj = FoxTalkType::CPtr(0x12345678);
        let buffer = &mut [0u8; 1024];


        let subj_pos = subj.write_data_to_buffer(buffer, size_of::<FoxtalkSize>());
        let pred_pos = pred.write_data_to_buffer(buffer, subj_pos.pos);
        let obj_pos = obj.write_data_to_buffer(buffer, pred_pos.pos);

        let size = obj_pos.pos as FoxtalkSize;
        let size_bytes = size.to_ne_bytes();

        buffer[0..size_of::<FoxtalkSize>()]
            .copy_from_slice(size_bytes.as_ref());

        let (parsed_subj, parsed_pred, parsed_obj) = parse_row(buffer);

        assert_eq!(subj, parsed_subj);
        assert_eq!(pred, parsed_pred);
        assert_eq!(obj, parsed_obj);
    }
}