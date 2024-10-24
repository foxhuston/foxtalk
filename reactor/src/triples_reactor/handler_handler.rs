use std::collections::HashSet;
use crate::reactor::Reactor;
use crate::reactor::reactor_handler::Handler;
use crate::triples_reactor::Tuple;

struct HandlerHandler<'a> {
    reactor: &'a Reactor<Tuple>
}

impl HandlerHandler<'_> {
    pub fn new<'r>(reactor: &'r Reactor<Tuple>) -> HandlerHandler<'r> {
        HandlerHandler { reactor }
    }
}

impl Handler<Tuple> for HandlerHandler<'_> {
    fn query(&mut self, o: &Tuple) -> bool {
        todo!()
    }

    fn handle(&mut self, input: &HashSet<Tuple>) -> HashSet<Tuple> {
        todo!()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    pub fn does_this_even_work() {
        let mut reactor: Reactor<Tuple> = Reactor::new();
        // reactor.add_handler(Box::new(HandlerHandler::new(&reactor)));
    }

}