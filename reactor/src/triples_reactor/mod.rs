pub mod serde;
pub mod ffi;

#[derive(Clone, Debug, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct Tuple(pub Vec<TupleNoun>);

#[derive(Debug, PartialEq, Clone, Eq, Hash)]
pub enum TupleNoun {
    Query,          // 0
    Symbol(String), // 1
    CPtr(u64),      // 2
    U64(u64),       // 3
    I64(i64),       // 4
}

#[cfg(test)]
mod tests {

}
