use std::collections::HashSet;
use std::hash::Hash;

pub trait Handler<O>
{
    fn query(&mut self, o: &O) -> bool;
    fn handle(&mut self, input: &HashSet<O>) -> HashSet<O>;

    fn free_o(&mut self, _o: &O) -> () {}
}

#[allow(non_snake_case)]
pub struct ReactorHandler<O: Eq + Hash>
{
    qa: Box<dyn Handler<O>>,
    pub(super) I: HashSet<O>,
    pub(super) O: HashSet<O>,
    pub(super) dirty: bool,
}

impl<O: Eq + Hash> ReactorHandler<O> {
    #[allow(non_snake_case)]
    pub fn new_with_bootstrap_output(qa: Box<dyn Handler<O>>, O: HashSet<O>) -> Self {
        Self {
            qa,
            O,
            I: HashSet::new(),
            dirty: false,
        }
    }

    pub fn new(qh: Box<dyn Handler<O>>) -> Self {
        Self::new_with_bootstrap_output(qh, HashSet::new())
    }

    pub fn query(&mut self, o: &O) -> bool {
        self.qa.query(o)
    }

    pub fn handle(&mut self) -> HashSet<O> {
        self.qa.handle(&self.I)
    }

    pub fn free_o(&mut self, o: &O) {
        self.qa.free_o(o)
    }
}
