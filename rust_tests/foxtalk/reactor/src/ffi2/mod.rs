pub mod c_heap_object;

/*
 * with cbindgen:
 */
use std::ffi::{c_void, CStr};
use std::ptr::NonNull;
use libc::c_char;
use crate::ffi2::c_heap_object::CHeapObject;
use crate::tuple::{Tuple, TupleNoun};

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
#[repr(C)]
pub struct PtrTuple {
    pub subject: *mut TupleNoun,
    pub predicate: *mut TupleNoun,
    pub object: *mut TupleNoun
}

///// TUPLE NOUN FFI CONSTRUCTORS //////////////////////////////////////////////

#[no_mangle]
pub extern "C" fn mk_tuple_noun_symbol(s: *const c_char) -> *mut TupleNoun {
    let sym = TupleNoun::Symbol(unsafe { CStr::from_ptr(s) }.to_owned().to_string_lossy().into_owned());
    Box::leak(Box::new(sym))
}

#[no_mangle]
pub extern "C" fn mk_tuple_noun_ptr(data: *mut c_void, free_fn: unsafe extern "C" fn(*mut c_void)) -> *mut TupleNoun {
    let tn = TupleNoun::CPtrWithFree(CHeapObject::new(data, free_fn));
    Box::leak(Box::new(tn))
}

#[no_mangle]
pub extern "C" fn mk_tuple_noun_query() -> *mut TupleNoun {
    Box::leak(Box::new(TupleNoun::Query()))
}
// ...

#[no_mangle]
pub extern "C" fn mk_tuple(subject: *mut TupleNoun, predicate: *mut TupleNoun, object: *mut TupleNoun) -> *mut PtrTuple {
    Box::leak(Box::new(PtrTuple { subject, predicate, object }))
}

///// TUPLE NOUN IMPORTERS /////////////////////////////////////////////////////

pub fn get_c_tuple_noun(tupn: *mut TupleNoun) -> Box<TupleNoun> {
    unsafe { Box::from_raw(tupn) }
}

#[no_mangle]
pub extern "C" fn get_tuple_subject(t: Tuple) -> TupleNoun { todo!() ; }
// ...

#[no_mangle]
pub extern "C" fn get_tuple_noun_as_string(tn: TupleNoun) -> *const c_char { todo!() ; }
// ...

///// TUPLE IMPORTERS //////////////////////////////////////////////////////////

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

///// UNIT TESTS ///////////////////////////////////////////////////////////////

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