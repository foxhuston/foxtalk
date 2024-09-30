

/*
 * with cbindgen:
 */
use std::ffi::{c_void, CStr};
use std::ptr::NonNull;
use libc::c_char;

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub enum TupleNoun {
    Query(),
    CPtrWithFree { data: NonNull<c_void>, destructor: NonNull<c_void> },
    Symbol(String),
    U64(u64),
    I64(i64)
}

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
#[repr(C)]
pub struct PtrTuple {
    pub subject: *mut TupleNoun,
    pub predicate: *mut TupleNoun,
    pub object: *mut TupleNoun
}

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub struct Tuple {
    pub subject: TupleNoun,
    pub predicate: TupleNoun,
    pub object: TupleNoun
}

#[no_mangle]
pub extern "C" fn mk_tuple_noun_symbol(s: *const c_char) -> *mut TupleNoun {
    let sym = TupleNoun::Symbol(unsafe { CStr::from_ptr(s) }.to_owned().to_string_lossy().into_owned());
    Box::leak(Box::new(sym))
}

// #[no_mangle]
// pub extern "C" fn mk_tuple_noun_ptr() -> TupleNoun { todo!() }

#[no_mangle]
pub extern "C" fn mk_tuple_noun_query() -> *mut TupleNoun {
    Box::leak(Box::new(TupleNoun::Query()))
}
// ...

#[no_mangle]
pub extern "C" fn mk_tuple(subject: *mut TupleNoun, predicate: *mut TupleNoun, object: *mut TupleNoun) -> *mut PtrTuple {
    Box::leak(Box::new(PtrTuple { subject, predicate, object }))
}

// #[no_mangle]
// pub extern "C" fn get_tuple_subject(t: Tuple) -> TupleNoun { todo!() ; }
// // ...
//
// #[no_mangle]
// pub extern "C" fn get_tuple_noun_as_string(tn: TupleNoun) -> *const c_char { todo!() ; }
// // ...

pub fn get_c_tuple_noun(tupn: *mut TupleNoun) -> Box<TupleNoun> {
    unsafe { Box::from_raw(tupn) }
}

pub fn get_c_tuple(tup: *mut PtrTuple) -> Tuple {
    let t = unsafe { Box::from_raw(tup) };
    let s = get_c_tuple_noun(t.subject);
    let p = get_c_tuple_noun(t.predicate);
    let o = get_c_tuple_noun(t.object);


    Tuple {
        subject: *s,
        predicate: *p,
        object: *o
    }
}

////////////////////////////////////////////////////////////////////////////////

#[cfg(test)]
mod test {
    use super::*;
    use libloading;

    #[test]
    fn it_should_copy_strings() {
        let lib = unsafe { libloading::Library::new("tests/test_libs/out/string_copy_test.so") }.unwrap();
        let get_query: libloading::Symbol<extern "C" fn () -> *mut PtrTuple> = unsafe { lib.get(b"GetQuery") }.unwrap();

        let q = get_c_tuple(get_query());
        println!("{:?}", q);

        match q.subject {
            TupleNoun::Symbol(s) => {
                assert_eq!(s, "description");
            }
            _ => { panic!("Unexpected Query Value") }
        }

    }
}