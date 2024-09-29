use std::ffi::c_void;
use std::mem;
use std::ptr::NonNull;

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
#[repr(C)]
pub enum TupleNoun {
    CPtrHeap { data: NonNull<c_void>, destructor: NonNull<c_void> },
    CPtr(NonNull<c_void>),
    Str(String),
}

type CFreeTuple = unsafe extern "C" fn(*mut c_void) -> ();
impl TupleNoun {

    pub(super) unsafe fn cleanup(&mut self) {
        match self {
            TupleNoun::CPtrHeap { data, destructor } => {
                // let d: CFreeTuple = unsafe { mem::transmute_copy(destructor) };
                // d(data.as_mut());
            }
            TupleNoun::CPtr(_) => {}
            TupleNoun::Str(_) => {}
        }
    }
}

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
#[repr(C)]
pub struct Tuple {
    pub subject: TupleNoun,
    pub predicate: String,
    pub object: TupleNoun
}

impl Tuple {
    pub fn new_strs(subject: &str, predicate: &str, object: &str) -> Tuple {
        Self::new_strings(subject.to_string(), predicate.to_string(), object.to_string())
    }

    pub fn new_strings(subject: String, predicate: String, object: String) -> Tuple {
        Tuple {
            subject: TupleNoun::Str(subject),
            predicate: predicate,
            object: TupleNoun::Str(object),
        }
    }

    pub unsafe fn cleanup(mut self) {
        // self.subject.cleanup();
        // self.object.cleanup();
    }
}