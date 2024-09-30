use std::ffi::{c_void, CStr, CString};
use std::ptr::NonNull;
use crate::bindings;
use crate::query::Query;
use crate::tuple::{Tuple, TupleNoun};

impl From<bindings::TupleNoun> for Option<TupleNoun> {
    fn from(value: bindings::TupleNoun) -> Self {
        unsafe {
            match value.tag {
                bindings::TupleNoun_Tag_Query => {
                    None
                }
                bindings::TupleNoun_Tag_Ptr => {
                    Some(TupleNoun::CPtr(NonNull::new(value.dat.ptr).unwrap()))
                }
                bindings::TupleNoun_Tag_Str => {
                    Some(TupleNoun::Str(CStr::from_ptr(value.dat.str_).to_string_lossy().into()))
                }
                bindings::TupleNoun_Tag_U64 => {
                    Some(TupleNoun::U64(value.dat.u64_))
                }
                bindings::TupleNoun_Tag_I64 => {
                    Some(TupleNoun::I64(value.dat.i64_))
                }

                _ => unreachable!()
            }
        }
    }
}

impl From<bindings::Tuple> for Query {
    fn from(value: bindings::Tuple) -> Self {
        let p = if value.predicate.is_null() { None } else { Some(unsafe { CStr::from_ptr(value.predicate) }) };
        let predicate = p.map(|s| s.to_str().unwrap().to_owned());

        Query {
            subject: value.subject.into(),
            predicate,
            object: value.object.into(),
        }
    }
}

impl Tuple {
    pub unsafe fn from_ctuple(free_subj: NonNull<c_void>, free_obj: NonNull<c_void>, value: bindings::Tuple) -> Self {
        assert_ne!(!value.subject.tag, bindings::TupleNoun_Tag_Query);
        assert_ne!(!value.object.tag, bindings::TupleNoun_Tag_Query);

        let s: Option<TupleNoun> = value.subject.into();
        let s = s.unwrap();
        let o: Option<TupleNoun> = value.object.into();
        let o = o.unwrap();

        let s = match s {
            TupleNoun::CPtr(p) => {
                TupleNoun::CPtrHeap { data: p, destructor: free_subj }
            }
            o => o
        };

        let p: String = unsafe { CStr::from_ptr(value.predicate).to_string_lossy().into() };

        let o = match o {
            TupleNoun::CPtr(p) => {
                TupleNoun::CPtrHeap { data: p, destructor: free_obj }
            }
            o => o
        };

        Tuple {
            subject: s,
            predicate: p,
            object: o,
        }
    }
}

impl From<TupleNoun> for bindings::TupleNoun {
    fn from(value: TupleNoun) -> Self {
        match value {
            TupleNoun::CPtr(data) |
            TupleNoun::CPtrHeap { data, destructor: _ } => {
                bindings::TupleNoun {
                    tag: bindings::TupleNoun_Tag_Ptr,
                    dat: bindings::TupleNoun_Dat {
                        ptr: data.as_ptr()
                    },
                }
            }

            TupleNoun::Str(s) => {
                bindings::TupleNoun {
                    tag: bindings::TupleNoun_Tag_Str,
                    dat: bindings::TupleNoun_Dat {
                        str_: CString::new(s).unwrap().into_raw()
                    },
                }
            }

            TupleNoun::U64(u) => {
                bindings::TupleNoun {
                    tag: bindings::TupleNoun_Tag_U64,
                    dat: bindings::TupleNoun_Dat {
                        u64_: u
                    }
                }
            }

            TupleNoun::I64(i) => {
                bindings::TupleNoun {
                    tag: bindings::TupleNoun_Tag_I64,
                    dat: bindings::TupleNoun_Dat {
                        i64_: i
                    }
                }
            }
        }
    }
}

