use crate::reactor::ReactorData;
use rustc_hash::FxHashSet;
use std::hash::Hash;

pub trait Program<O: ReactorData, Q> where Self: Send
{
    fn query(&mut self) -> Q;
    fn handle(&mut self, input: &FxHashSet<O>) -> FxHashSet<O>;

    fn poll(&mut self) -> bool { false  }

    fn free_o(&mut self, _o: &O) -> () {}

    // fn as_any(&self) -> &dyn Any { self }
}


#[allow(non_snake_case)]
pub struct ProgramInfo<O: Eq + Hash>
{
    pub(super) I: FxHashSet<O>,

    pub(super) O: FxHashSet<O>,

    pub(super) dirty: bool,
}

impl<O: Eq + Hash> Default for ProgramInfo<O> {
    fn default() -> Self {
        Self {
            I: FxHashSet::default(),
            O: FxHashSet::default(),
            dirty: false,
        }
    }
}
