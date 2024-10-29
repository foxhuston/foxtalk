use crate::reactor::{ReactorData, ReactorHandlerId};

pub trait QueryEngine<O, P, Q> {
    fn insert_program_for_query(&mut self, q: Q, p: P) -> ();
    fn remove_program(&mut self, q: Q, p: P) -> ();
    fn query(&mut self, o: O) -> Vec<P>;
}

