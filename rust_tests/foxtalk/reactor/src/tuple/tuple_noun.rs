use std::rc::Rc;
use crate::ffi2::c_heap_object::CHeapObject;

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
pub enum TupleNoun {
    Query(), // TODO: get rid of ()? Will the ffi freak out?
    CPtrWithFree(CHeapObject),
    Symbol(String),
    U64(u64),
    I64(i64)
}

impl Drop for TupleNoun {
    fn drop(&mut self) {
        println!("Dropping TupleNoun {self:?}!");
    }
}

impl TupleNoun {
    pub fn is_query(&self) -> bool {
        match self {
            TupleNoun::Query() => true,
            _ => false
        }
    }

    pub fn mk_panic_msg(&self, should_be: &'static str) -> String {
        match self {
            TupleNoun::Query() => { format!("Expected TupleNoun to be {should_be}, but it was actually a Query!") }
            TupleNoun::CPtrWithFree(_) => { format!("Expected TupleNoun to be {should_be}, but it was actually a CPtrWithFree!") }
            TupleNoun::Symbol(_) => { format!("Expected TupleNoun to be {should_be}, but it was actually a Symbol!") }
            TupleNoun::U64(_) => { format!("Expected TupleNoun to be {should_be}, but it was actually a U64!") }
            TupleNoun::I64(_) => { format!("Expected TupleNoun to be {should_be}, but it was actually an I64!") }
        }
    }
}
