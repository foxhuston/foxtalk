use crate::reactor::ReactorData;
use std::collections::HashSet;
use std::hash::Hash;

pub trait Program<O: ReactorData> where Self: Send
{
    fn handle(&mut self, input: &HashSet<O>) -> HashSet<O>;

    fn free_o(&mut self, _o: &O) -> () {}
}


#[allow(non_snake_case)]
pub struct ProgramInfo<O: Eq + Hash>
{
    pub(super) I: HashSet<O>,
    pub(super) O: HashSet<O>,
    pub(super) dirty: bool,
}