use std::sync::Arc;
use tuple_db::tuple::Tuple;


// TODO: `When` should give the entire results set, and then I can write a
//       `WhenEach` that does the  behavior of looping through each of the tuples.
//       This way, if a handler needs to do some kind of reduction (like, pick the best camera
//       resolution, for instance), it can.
pub trait When {
    fn get_query(&self) -> Arc<Tuple>;

    // TODO: The results might be more than one tuple, particularly in the case of conjunction.
    //       It might be better to actually just give it a list of `TupleNoun`s for the
    //       variable positions...?
    fn handle(&mut self, results: Arc<Tuple>) -> Vec<Arc<Tuple>>;
}