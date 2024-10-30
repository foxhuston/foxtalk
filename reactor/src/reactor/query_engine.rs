use std::hash::Hash;

pub trait QueryEngine<O, P, Q> {
    fn insert_program_for_query(&mut self, q: Q, p: P) -> ();
    fn remove_program(&mut self, q: Q, p: P) -> ();
    fn query(&mut self, o: &O) -> Vec<P>;
}

pub(crate) struct SimpleQueryEngine<P, Q> {
    pub(crate) programs: Vec<(Q, P)>
}

impl<P, Q> SimpleQueryEngine<P, Q> {
    pub fn new() -> Self {
        Self {
            programs: Vec::new()
        }
    }
}

pub trait SimpleQuery<O> {
    fn query(&self, o: &O) -> bool;
}

impl<O, P: Hash + Eq + Clone, Q: SimpleQuery<O> + Eq> QueryEngine<O, P, Q> for SimpleQueryEngine<P, Q> {
    fn insert_program_for_query(&mut self, q: Q, p: P) -> () {
        self.programs.push((q, p));
    }

    fn remove_program(&mut self, q: Q, p: P) -> () {
        self.programs
            .retain(|(query, program)| { !(*query == q && *program == p) });
    }

    fn query(&mut self, o: &O) -> Vec<P> {
        self.programs.iter()
            .filter(|(query, _)| { query.query(&o) })
            .map(|(_, program)| program)
            .cloned()
            .collect()
    }
}

