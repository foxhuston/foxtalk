pub mod c_heap_object;
pub mod c_when;

pub use c_when::*;

use std::ffi::{c_void, CStr, CString};
use libc::c_char;
use ustr::Ustr;
use crate::ffi2::c_heap_object::CHeapObject;
use crate::tuple::{Tuple, TupleNoun};

///// PtrTuple FFI OBJECT //////////////////////////////////////////////////////

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
#[repr(C)]
pub struct PtrTuple {
    pub subject: *mut TupleNoun,
    pub predicate: *mut TupleNoun,
    pub object: *mut TupleNoun
}

///// TUPLE NOUN FFI CONSTRUCTORS //////////////////////////////////////////////
#[no_mangle]
pub extern "C" fn mk_tuple_noun_query() -> *mut TupleNoun {
    Box::leak(Box::new(TupleNoun::Query()))
}

#[no_mangle]
pub extern "C" fn mk_tuple_noun_cptr_with_free(data: *mut c_void, free_fn: unsafe extern "C" fn(*mut c_void)) -> *mut TupleNoun {
    let tn = TupleNoun::CPtrWithFree(CHeapObject::new(data, free_fn));
    Box::leak(Box::new(tn))
}

#[no_mangle]
pub extern "C" fn mk_tuple_noun_symbol(s: *const c_char) -> *mut TupleNoun {
    let us = Ustr::from(unsafe { CStr::from_ptr(s) }.to_str().unwrap());
    let sym = TupleNoun::Symbol(us);
    Box::leak(Box::new(sym))
}

#[no_mangle]
pub extern "C" fn mk_tuple_noun_u64(n: u64) -> *mut TupleNoun {
    Box::leak(Box::new(TupleNoun::U64(n)))
}

#[no_mangle]
pub extern "C" fn mk_tuple_noun_i64(n: i64) -> *mut TupleNoun {
    Box::leak(Box::new(TupleNoun::I64(n)))
}

///// TUPLE NOUN FFI DATA READERS //////////////////////////////////////////////

#[no_mangle]
pub extern "C" fn get_tuple_noun_as_symbol(tuple_noun: *mut TupleNoun) -> *const c_char {
    match unsafe { &*tuple_noun } {
        TupleNoun::Symbol(s) => {
            s.as_char_ptr()
        }
        s => panic!("{}", s.mk_panic_msg("a Symbol"))
    }
}

#[no_mangle]
pub extern "C" fn get_tuple_noun_as_cptr(tuple_noun: *mut TupleNoun) -> *mut c_void {
    match unsafe { &*tuple_noun } {
        TupleNoun::CPtrWithFree(d) => { d.data() }
        s => panic!("{}", s.mk_panic_msg("a CPtrWithFree"))
    }
}

#[no_mangle]
pub extern "C" fn get_tuple_noun_as_u64(tuple_noun: *mut TupleNoun) -> u64 {
    match unsafe { &*tuple_noun } {
        TupleNoun::U64(n) => { *n }
        s => panic!("{}", s.mk_panic_msg("a U64"))
    }
}

#[no_mangle]
pub extern "C" fn get_tuple_noun_as_i64(tuple_noun: *mut TupleNoun) -> i64 {
    match unsafe { &*tuple_noun } {
        TupleNoun::I64(n) => { *n }
        s => panic!("{}", s.mk_panic_msg("an I64"))
    }
}

///// TUPLE FFI CONSTRUCTOR ////////////////////////////////////////////////////

#[no_mangle]
pub extern "C" fn mk_tuple(subject: *mut TupleNoun, predicate: *mut TupleNoun, object: *mut TupleNoun) -> *mut PtrTuple {
    Box::leak(Box::new(PtrTuple { subject, predicate, object }))
}

///// TUPLE NOUN DATA READERS //////////////////////////////////////////////////
#[no_mangle]
pub extern "C" fn get_tuple_subject(t: *mut PtrTuple) -> *mut TupleNoun {
    unsafe { (*t).subject }
}

#[no_mangle]
pub extern "C" fn get_tuple_predicate(t: *mut PtrTuple) -> *mut TupleNoun {
    unsafe { (*t).predicate }
}

#[no_mangle]
pub extern "C" fn get_tuple_object(t: *mut PtrTuple) -> *mut TupleNoun {
    unsafe { (*t).object }
}

///// TUPLE NOUN IMPORTER //////////////////////////////////////////////////////

pub fn get_c_tuple_noun(tupn: *mut TupleNoun) -> Box<TupleNoun> {
    unsafe { Box::from_raw(tupn) }
}

///// TUPLE IMPORTER ///////////////////////////////////////////////////////////

pub fn get_c_tuple(tup: *mut PtrTuple) -> Tuple {
    let t = unsafe { Box::from_raw(tup) };
    let s = get_c_tuple_noun(t.subject);
    let p = get_c_tuple_noun(t.predicate);
    let o = get_c_tuple_noun(t.object);


    let out = Tuple {
        subject: *s,
        predicate: *p,
        object: *o
    };

    // So what's happening here is that in CPP, we pass the pointer to a `TupleNoun`
    // we got from the query right back into `mk_tuple` as the subject (for instance).
    // So we get a double-free, because in this function, `get_c_tuple_noun` always takes
    // ownership back, which means that some tuple-noun somewhere will get dropped twice.
    println!("get_c_tuple: {out:?}");
    out
}

impl From<*mut PtrTuple> for Tuple {
    fn from(value: *mut PtrTuple) -> Self {
        get_c_tuple(value)
    }
}

impl From<Tuple> for *mut PtrTuple {
    fn from(value: Tuple) -> Self {
        Box::leak(Box::new(PtrTuple {
            subject: Box::leak(Box::new(value.subject)),
            predicate: Box::leak(Box::new(value.predicate)),
            object: Box::leak(Box::new(value.object)),
        }))
    }
}

///// UNIT TESTS ///////////////////////////////////////////////////////////////

#[cfg(test)]
mod test {
    use super::*;
    use libloading;
    use crate::reactor::Reactor;
    use crate::tuple::test_helpers;

    #[test]
    fn tuple_noun_string() {
        let lib = unsafe { libloading::Library::new("tests/test_libs/out/string_copy_test.so") }.unwrap();
        let get_query: libloading::Symbol<extern "C" fn () -> *mut PtrTuple> = unsafe { lib.get(b"GetQuery") }.unwrap();

        let q = get_c_tuple(get_query());
        println!("{:?}", q);

        // match q.subject {
        //     TupleNoun::Symbol(s) => {
        //         assert_eq!(s, "description");
        //     }
        //     _ => { panic!("Unexpected Query Value") }
        // }
    }

    #[test]
    fn regression_double_free() {
        let mut reactor = Reactor::new();
        reactor.claim(test_helpers::mk_tuple("lexi", "is a", "husky"));

        let cwhen_a = unsafe { CWhen::new("tests/test_libs/out/regression_double_free.so").unwrap() };

        reactor.add_handler(Box::new(cwhen_a));

        reactor.tick();
    }
}