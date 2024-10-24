use std::collections::HashSet;
use std::hash::Hash;

pub trait Handler<O>
{
    fn query(&mut self, o: &O) -> bool;
    fn handle(&mut self, input: &HashSet<O>) -> HashSet<O>;
}

#[allow(non_snake_case)]
pub struct ReactorHandler<O: Eq + Hash>
{
    pub(crate) qa: Box<dyn Handler<O>>,
    pub(crate) I: HashSet<O>,
    pub(crate) O: HashSet<O>,
    pub(crate) dirty: bool,
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
}
