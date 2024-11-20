
pub use std::ptr::slice_from_raw_parts_mut;
pub use lazy_static::lazy_static;
pub use log::error;

pub use rust_tuple_reactor_serde::tuple::Tuple;
pub use rust_tuple_reactor_serde::*;

pub trait NeedsToImplement {
    fn query(&self) -> Vec<Tuple>;
    fn initial_tuples(&self) -> Option<Vec<Tuple>> {
        None
    }
    fn handle(&self, tuples: Vec<Tuple>) -> Option<Vec<Tuple>>;
    fn free_tuple(&self, _: Tuple) -> () {}
    fn teardown(&self) -> () {}
    fn poll(&self) -> bool {
        false
    }
    fn new() -> Self;
}

#[macro_export]
macro_rules! foxtalk_handler {
    ($struct_name:ident) => {
        lazy_static! {
            static ref HANDLER: $struct_name = $struct_name::new();
        }



        static BUFFER_SIZE: usize = 10 * 1024 * 1024;

        #[no_mangle]
        pub extern "C" fn foxtalk_init(buffer: *mut u8) {
            let tuples = HANDLER.query();
            unsafe {
                match slice_from_raw_parts_mut(buffer, BUFFER_SIZE).as_mut() {
                    None => { error!("Slice from raw parts failed") }
                    Some(buf) => { tuples.iter().write_to_buffer(buf, 0); }
                }
            };
        }
        #[no_mangle]
        pub extern "C" fn foxtalk_free_tuple(buffer: *mut u8) {
            unsafe {
                match slice_from_raw_parts_mut(buffer, BUFFER_SIZE).as_mut() {
                    None => { eprintln!("Slice from raw parts failed") }
                    Some(buf) => {
                        let (tuple, _) = Tuple::read_from_buffer(buf, 0);
                        HANDLER.free_tuple(tuple);
                    }
                }
            };
        }
        #[no_mangle]
        pub extern "C" fn foxtalk_handle(buffer: *mut u8) {
            unsafe {
                match slice_from_raw_parts_mut(buffer, BUFFER_SIZE).as_mut() {
                    None => { eprintln!("Slice from raw parts failed") }
                    Some(buf) => {
                        let (tuples, _) = unsafe { Vec::<Tuple>::read_from_buffer(buf, 0) };

                        // clear buffer
                        let v: &[Tuple] = &vec![];
                        v.iter().write_to_buffer(buf, 0);

                        if let Some(claims) = HANDLER.handle(tuples) {
                            claims.iter().write_to_buffer(buf, 0);
                        }
                    }
                }
            };
        }
        #[no_mangle]
        pub extern "C" fn foxtalk_register_initial_tuples(buffer: *mut u8) {
            if let Some(tuples) = HANDLER.initial_tuples() {
                unsafe {
                    match slice_from_raw_parts_mut(buffer, BUFFER_SIZE).as_mut() {
                        None => { error!("Slice from raw parts failed") }
                        Some(buf) => { tuples.iter().write_to_buffer(buf, 0); }
                    }
                };
            }
        }
        #[no_mangle]
        pub extern "C" fn foxtalk_teardown() {
            HANDLER.teardown()
        }
        #[no_mangle]
        pub extern "C" fn foxtalk_poll() -> bool {
            HANDLER.poll()
        }



    };
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn it_compiles() {

        struct MyImpl;

        impl NeedsToImplement for MyImpl {
            fn query(&self) -> Vec<Tuple> {
                todo!()
            }

            fn handle(&self, _: Vec<Tuple>) -> Option<Vec<Tuple>> {
                todo!()
            }

            fn new() -> Self {
                Self {}
            }

        }

        foxtalk_handler!(MyImpl);

    }
}