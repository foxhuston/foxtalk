pub mod c_heap_object;
pub mod c_when;

pub use c_when::*;

use libc::c_char;
use std::ffi::CStr;
use std::sync::Arc;
use tuple_db::tuple::{Tuple, TupleNoun};
use ustr::Ustr;

///// PtrTuple FFI OBJECT //////////////////////////////////////////////////////

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
#[repr(C)]
pub struct PtrTuple {
    pub subject: *mut TupleNoun,
    pub predicate: *mut TupleNoun,
    pub object: *mut TupleNoun
}

impl PtrTuple {
    pub fn to_tuple(&self) -> Arc<Tuple> {
        Arc::new(Tuple {
            subject: unsafe { Arc::from_raw(self.subject) },
            predicate: unsafe { Arc::from_raw(self.predicate) },
            object: unsafe { Arc::from_raw(self.object) }
        })
    }

    pub fn from_tuple(t: Arc<Tuple>) -> *mut Self {
        let subject = Arc::into_raw(t.subject.to_owned()).cast_mut();
        let predicate = Arc::into_raw(t.predicate.to_owned()).cast_mut();
        let object = Arc::into_raw(t.object.to_owned()).cast_mut();

        &mut PtrTuple {
            subject,
            predicate,
            object
        }

    }
}

///// TUPLE NOUN FFI CONSTRUCTORS //////////////////////////////////////////////
#[no_mangle]
pub extern "C" fn mk_tuple_noun_query() -> *mut TupleNoun {
    Arc::into_raw(Arc::new(TupleNoun::Query)).cast_mut()
}

// #[no_mangle]
// pub extern "C" fn mk_tuple_noun_cptr_with_free(data: *mut c_void, free_fn: unsafe extern "C" fn(*mut c_void)) -> *mut TupleNoun {
//     let tn = TupleNoun::CPtrWithFree(CHeapObject::new(data, free_fn));
//     Box::leak(Box::new(tn))
// }

#[no_mangle]
pub extern "C" fn mk_tuple_noun_symbol(s: *const c_char) -> *mut TupleNoun {
    let us = Ustr::from(unsafe { CStr::from_ptr(s) }.to_str().unwrap());
    let sym = Arc::into_raw(Arc::new(TupleNoun::Symbol(us)));
    sym.cast_mut()
}

#[no_mangle]
pub extern "C" fn mk_tuple_noun_u64(n: u64) -> *mut TupleNoun {
    &mut TupleNoun::U64(n)
}

#[no_mangle]
pub extern "C" fn mk_tuple_noun_i64(n: i64) -> *mut TupleNoun {
    &mut TupleNoun::I64(n)
}

///// TUPLE NOUN FFI DATA READERS //////////////////////////////////////////////

#[no_mangle]
pub extern "C" fn get_tuple_noun_as_symbol(tuple_noun: *mut TupleNoun) -> *const c_char {
    match unsafe { Arc::from_raw(tuple_noun) }.as_ref() {
        TupleNoun::Symbol(s) => {
            s.as_char_ptr()
        }
        s => panic!("{}", s.mk_panic_msg("a Symbol"))
    }
}

// #[no_mangle]
// pub extern "C" fn get_tuple_noun_as_cptr(tuple_noun: *mut TupleNoun) -> *mut c_void {
//     match unsafe { &*tuple_noun } {
//         TupleNoun::CPtrWithFree(d) => { d.data() }
//         s => panic!("{}", s.mk_panic_msg("a CPtrWithFree"))
//     }
// }

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
pub extern "C" fn mk_tuple<'a>(subject: *mut TupleNoun, predicate: *mut TupleNoun, object: *mut TupleNoun) -> &'a PtrTuple {
    println!("Making tuple <{subject:?}, {predicate:?}, {object:?}>...");
    let out = PtrTuple { subject, predicate, object };
    println!("Made tuple {out:?}");
    Box::leak(Box::new(out))
}

///// TUPLE NOUN DATA READERS //////////////////////////////////////////////////
#[no_mangle]
pub extern "C" fn get_tuple_subject(t: *mut PtrTuple) -> *mut TupleNoun {

    unsafe { Arc::from_raw(t.cast_const()) }.subject
    // unsafe { (*t).subject }
}

#[no_mangle]
pub extern "C" fn get_tuple_predicate(t: *mut PtrTuple) -> *mut TupleNoun {
    unsafe { Arc::from_raw(t.cast_const()) }.predicate
}

#[no_mangle]
pub extern "C" fn get_tuple_object(t: *mut PtrTuple) -> *mut TupleNoun {
    unsafe { Arc::from_raw(t.cast_const()) }.object
}

///// TUPLE NOUN IMPORTER //////////////////////////////////////////////////////

pub fn get_c_tuple_noun(tupn: *mut TupleNoun) -> Arc<TupleNoun> {
    unsafe { Arc::from_raw(tupn) }
}

///// TUPLE IMPORTER ///////////////////////////////////////////////////////////

pub fn get_c_tuple(tup: *mut PtrTuple) -> Tuple {
    let t = unsafe { Arc::from_raw(tup) };
    let s = get_c_tuple_noun(t.subject);
    let p = get_c_tuple_noun(t.predicate);
    let o = get_c_tuple_noun(t.object);


    let out = Tuple {
        subject: s,
        predicate: p,
        object: o
    };

    // So what's happening here is that in CPP, we pass the pointer to a `TupleNoun`
    // we got from the query right back into `mk_tuple` as the subject (for instance).
    // So we get a double-free, because in this function, `get_c_tuple_noun` always takes
    // ownership back, which means that some tuple-noun somewhere will get dropped twice.
    println!("get_c_tuple: {out:?}");
    out
}

// impl From<*mut PtrTuple> for Tuple {
//     fn from(value: *mut PtrTuple) -> Self {
//         get_c_tuple(value)
//     }
// }
//
// impl From<Tuple> for *mut PtrTuple {
//     fn from(value: Tuple) -> Self {
//         Box::leak(Box::new(PtrTuple {
//             subject: Box::leak(Box::new(value.subject)),
//             predicate: Box::leak(Box::new(value.predicate)),
//             object: Box::leak(Box::new(value.object)),
//         }))
//     }
// }

///// UNIT TESTS ///////////////////////////////////////////////////////////////

#[cfg(test)]
mod test {
    use super::*;
    use crate::reactor::Reactor;
    use libloading;
    use tuple_db::tuple::test_helpers;

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