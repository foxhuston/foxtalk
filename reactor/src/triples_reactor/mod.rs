use serde::TupleNoun;

pub mod serde;
pub mod ffi;

#[derive(Clone, Debug, Eq, PartialEq, Hash)]
#[repr(transparent)]
pub struct Tuple(pub Vec<TupleNoun>);

#[cfg(test)]
mod tests {

}
