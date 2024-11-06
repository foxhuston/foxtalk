use std::collections::{HashMap, HashSet};
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;
use crate::triples_reactor::{Tuple};

struct TupleQuery {}

impl TupleQuery {
    pub fn new(s: &str) -> Self {
        TupleQuery {}
    }

    /// Blah
    ///
    /// For example: the query `(you) has width /width/` requires the input-binding
    /// `you -> <some noun>`, and will generate a map with `width -> <some noun>` if the
    /// query matches. It will return `None` if it doesn't.
    pub fn run(&self, input_bindings: HashMap<String, TupleNoun>, tuples: &HashSet<Tuple>) -> Option<HashMap<String, TupleNoun>> {
        todo!()
    }

    // TODO: This needs to partially accumulate for conjunction, and notify the reactor if it actually
    //       cares about something that's part of one of its subclauses.
}

#[cfg(test)]
pub mod test {
    use nom::branch::alt;
    use nom::multi::many_till;
    use nom::bytes::complete::{tag, take_till};
    use nom::character::complete::anychar;
    use nom::IResult;
    use nom::sequence::delimited;
    use crate::triples_reactor::query::TupleQuery;

    fn vlookup(input: &str) -> IResult<&str, String> {
        let (a, (b, _)) = delimited(tag("("), many_till(tag(")"), anychar), tag(")"))(input)?;
        
        Ok((a, b.into()))
    }

    #[test]
    pub fn learning_nom() {
        let q = "(you) has width /width/";
        
        println!("{:?}", vlookup("(you)"))

        // let vbind = delimited(tag("/"), anychar, tag("/"));
        // 
        // let parser = alt((vlookup, vbind));

        // match parser(q) {
        // 
        // }

    }
}



















