use std::collections::{HashMap, HashSet};
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;
use crate::triples_reactor::{Tuple};
use anyhow::Result;

use combine::{many1, Parser, sep_by, choice, between, token};
use combine::parser::char::{letter, space};

#[derive(Debug, PartialEq)]
enum QueryExpr<A> {
    EIdent(A),
    // EI64, EU64, EDouble (eventually) etc...
    ELookup(A),
    EBinding(A),
    ETuple(Vec<QueryExpr<A>>),
    EBoundLit(A, Box<QueryExpr<A>>),
    EOr(Box<QueryExpr<A>>, Box<QueryExpr<A>>),
    EAnd(Box<QueryExpr<A>>, Box<QueryExpr<A>>),
}

impl QueryExpr<String> {
    pub fn new(inp: &str) -> Result<QueryExpr<String>> {
        let word = many1(letter());
        let ident = word.map(|c: String| QueryExpr::EIdent(c));
        let binding = between(token('/'), word, token('/')).map(|c: String| QueryExpr::EBinding(c));
        
        let mut parser = choice([ident]);

        let (out, _) = parser.parse(inp)?;
        Ok(out)
    }
}


#[cfg(test)]
pub mod test {
    use crate::triples_reactor::query::QueryExpr;

    #[test]
    pub fn learning_combine() {
        let qe= QueryExpr::new("bork").unwrap();
        assert_eq!(qe, QueryExpr::EIdent("bork".to_string()));

        // let vbind = delimited(tag("/"), anychar, tag("/"));
        // 
        // let parser = alt((vlookup, vbind));

        // match parser(q) {
        // 
        // }

    }
}



















