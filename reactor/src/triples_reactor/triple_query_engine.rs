use crate::reactor::query_engine::QueryEngine;
use crate::triples_reactor::{Tuple, TupleNoun};
use std::collections::HashMap;

struct TripleQueryEngine<P>{
    flat_node_tree: Vec<TripleQueryNode<P>>
}

impl<P> TripleQueryEngine<P> {
    pub fn new() -> Self {
        TripleQueryEngine {
            flat_node_tree: vec![TripleQueryNode::new()]
        }
    }
}

struct TripleQueryNode<P>{
    children: HashMap<TupleNoun, usize>,
    pub matched_prefix_programs: Vec<P>,
    pub matched_exact_programs: Vec<P>,
}

impl<P> TripleQueryNode<P> {
    pub fn new() -> Self {
        TripleQueryNode {
            children: HashMap::new(),
            matched_prefix_programs: Vec::new(),
            matched_exact_programs: Vec::new()
        }
    }
}


impl<P> QueryEngine<Tuple, P, Tuple> for TripleQueryEngine<P> {
    fn insert_program_for_query(&mut self, Tuple(nouns): Tuple, p: P) -> () {
        let mut current_node = &mut self.flat_node_tree[0];
        for n in nouns {
            let child_index = match current_node.children.get(&n) {
                // Some(i) => *i,
                // None => {
                //     let new_node = TripleQueryNode::new();
                //     self.flat_node_tree.push(new_node);
                //     let new_index = self.flat_node_tree.len() - 1;
                //     current_node.children.insert(n, new_index);
                //     new_index
                // }
            };
            current_node = &mut self.flat_node_tree[child_index];
        }
    }

    fn remove_program(&mut self, p: P) -> () {
        todo!()
    }

    fn query(&mut self, Tuple(nouns): Tuple) -> Vec<P> {
        
        let mut output_programs = Vec::new();
        
        for n in nouns {
            // start at root
            // if root has noun branch that matches n, go down that branch
            // AND also always go down the ? branch
        }
        
        
        output_programs
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    pub fn paper_example_2() {
        // Q: <lexi, is a, husky>
        let mut engine = TripleQueryEngine::new();
        engine.insert_program_for_query(Tuple::triple_from_sss("lexi", "is a", "husky"), 1);
        
        let actual = engine.query(Tuple::triple_from_sss("lexi", "is a", "husky"));
        assert_eq!(actual, vec![1]);
    }
}