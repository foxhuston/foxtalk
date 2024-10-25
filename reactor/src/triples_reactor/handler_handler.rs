use std::collections::HashSet;
use crate::reactor::Reactor;
use crate::reactor::reactor_handler::Handler;
use crate::triples_reactor::Tuple;
use crate::reactor::utils::non_aggregating_handler::*;
struct HandlerHandler<'a> {
    reactor: &'a Reactor<Tuple>
}

impl HandlerHandler<'_> {
    pub fn new<'r>(reactor: &'r Reactor<Tuple>) -> HandlerHandler<'r> {
        HandlerHandler { reactor }
    }
}

impl NonAggregatingHandler<Tuple> for HandlerHandler<'_> {
    fn query(&mut self, o: &Tuple) -> bool {
        // match tuple: <?, is a, handler>
        // ?: path to a .so file
        todo!()
    }

    fn handle(&mut self, input: Tuple) -> HashSet<Tuple> {
        // grab first noun
        // create new dynamichandler
        // register with reactor <--- impossible with Rust?
        todo!()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    pub fn does_this_even_work() {
        let mut reactor: Reactor<Tuple> = Reactor::new();
        let handler = HandlerHandler::new(&reactor);
        let adapter = NonAggregatingAdapter::new(Box::new(handler));
        reactor.add_handler(Box::new(adapter));
    }

}