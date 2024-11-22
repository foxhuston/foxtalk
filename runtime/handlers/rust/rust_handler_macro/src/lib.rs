
pub use std::ptr::slice_from_raw_parts_mut;
pub use lazy_static::lazy_static;
pub use log::error;

pub use rust_tuple_reactor_serde::tuple::Tuple;
pub use rust_tuple_reactor_serde::*;

pub trait NeedsToImplement {
    fn query(&mut self) -> Vec<Tuple>;
    fn initial_tuples(&mut self) -> Option<Vec<Tuple>> {
        None
    }
    fn handle(&mut self, tuples: Vec<Tuple>) -> Vec<Tuple>;
    fn free_tuple(&mut self, _: Tuple) -> () {}
    fn teardown(&mut self) -> () {}
    fn poll(&mut self) -> bool {
        false
    }
    fn new() -> Self;
}

#[macro_export]
macro_rules! foxtalk_handler {
    ($struct_name:ident) => {
        lazy_static! {
            static ref HANDLER: Arc<Mutex<$struct_name>> = Arc::new(Mutex::new($struct_name::new()));
        }
        static BUFFER_SIZE: usize = 10 * 1024 * 1024;
        #[no_mangle]
        pub extern "C" fn foxtalk_init(buffer: *mut u8) {
            let mut hlock = HANDLER.lock();
            let tuples = hlock.query();
            unsafe {
                match slice_from_raw_parts_mut(buffer, BUFFER_SIZE).as_mut() {
                    None => { error!( "Slice from raw parts failed" ) }
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
                        let mut hlock = HANDLER.lock();
                        hlock.free_tuple(tuple);
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


                        let v: &[Tuple] = &vec![];
                        v.iter().write_to_buffer(buf, 0);

                        let mut hlock = HANDLER.lock();
                        let claims = hlock.handle(tuples);
                        claims.iter().write_to_buffer(buf, 0);
                    }
                }
            };
        }
        #[no_mangle]
        pub extern "C" fn foxtalk_register_initial_tuples(buffer: *mut u8) {
            let mut hlock = HANDLER.lock();
            if let Some(tuples) = hlock.initial_tuples() {
                unsafe {
                    match slice_from_raw_parts_mut(buffer, BUFFER_SIZE).as_mut() {
                        None => { error!( "Slice from raw parts failed" ) }
                        Some(buf) => { tuples.iter().write_to_buffer(buf, 0); }
                    }
                };
            }
        }
        #[no_mangle]
        pub extern "C" fn foxtalk_teardown() {
            let mut hlock = HANDLER.lock();
            hlock.teardown()
        }
        #[no_mangle]
        pub extern "C" fn foxtalk_poll() -> bool {
            let mut hlock = HANDLER.lock();
            hlock.poll()
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