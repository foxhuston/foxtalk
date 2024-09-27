use crate::query::Query;
use crate::tuple::Tuple;


pub trait When {
    fn get_query(&self) -> Query;

    // TODO: The results might be more than one tuple, particularly in the case of conjunction.
    //       It might be better to actually just give it a list of `TupleNoun`s for the
    //       variable positions...?
    fn handle(&mut self, wish: &mut dyn FnMut(Tuple), results: Tuple) -> ();
}