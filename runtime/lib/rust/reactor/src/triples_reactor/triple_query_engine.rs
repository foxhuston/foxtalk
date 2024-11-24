use crate::reactor::query_engine::QueryEngine;
use std::collections::{HashMap, VecDeque};
use std::fmt::Debug;
use rust_tuple_reactor_serde::tuple::Tuple;
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;

pub struct TripleQueryEngine<P> {
    flat_node_tree: Vec<TripleQueryNode<P>>,
}

impl<P: PartialEq> TripleQueryEngine<P> {
    pub fn new() -> Self {
        TripleQueryEngine {
            flat_node_tree: vec![TripleQueryNode::new()]
        }
    }

    fn mk_node(&mut self) -> usize {
        self.flat_node_tree.push(TripleQueryNode::new());
        self.flat_node_tree.len() - 1
    }

    fn get_children(&self, id: usize) -> &HashMap<TupleNoun, usize> {
        &self.flat_node_tree.get(id).unwrap().children
    }
}

struct TripleQueryNode<P> {
    children: HashMap<TupleNoun, usize>,
    pub matched_prefix_programs: Vec<P>,
    pub matched_exact_programs: Vec<P>,
}


impl<O: Debug> Debug for TripleQueryEngine<O> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let mut work_queue: VecDeque<(usize, Vec<String>)> = VecDeque::new();
        work_queue.push_back((0, vec![]));

        while let Some((np, path)) = work_queue.pop_back() {
            let node = self.flat_node_tree.get(np).unwrap();

            let _ = writeln!(f, "{}: <{:?}, {:?}>", path.join(" / "), node.matched_prefix_programs, node.matched_exact_programs);

            let node_children = &self.flat_node_tree.get(np).unwrap().children;
            for (noun, &child) in node_children {
                work_queue.push_back((child, [path.clone(), vec![format!("{noun:?}")]].concat()))
            }
            
        }

        Ok(())
    }
}


impl<P: PartialEq> TripleQueryNode<P> {
    pub fn new() -> Self {
        TripleQueryNode {
            children: HashMap::new(),
            matched_prefix_programs: Vec::new(),
            matched_exact_programs: Vec::new(),
        }
    }

    pub fn remove_from_prefix_programs(&mut self, p: P) {
        self.matched_prefix_programs.retain(|v| *v != p)
    }
    pub fn remove_from_exact_programs(&mut self, p: P) {
        self.matched_exact_programs.retain(|v| *v != p)
    }
}


impl<P: Clone + PartialEq> QueryEngine<Tuple, P, Vec<Tuple>> for TripleQueryEngine<P> {
    fn insert_program_for_query(&mut self, query_tuples: Vec<Tuple>, p: P) -> () {
        for Tuple(nouns) in query_tuples {

            let mut is_prefix = false;
            let p = p.clone();
            let mut current_node_id: usize = 0;

            for n in nouns {
                match n {
                    TupleNoun::Prefix => {
                        is_prefix = true;
                        break;
                    }

                    n => {
                        let next = self.get_children(current_node_id).get(&n);
                        match next {
                            Some(&next_id) => { current_node_id = next_id }
                            None => {
                                let next_node_id = self.mk_node();
                                self.flat_node_tree.get_mut(current_node_id).unwrap().children.insert(n, next_node_id);
                                current_node_id = next_node_id;
                            }
                        }
                    }
                }
            }
            if is_prefix {
                self.flat_node_tree.get_mut(current_node_id).unwrap().matched_prefix_programs.push(p);
            } else {
                self.flat_node_tree.get_mut(current_node_id).unwrap().matched_exact_programs.push(p);
            }
        }
    }

    fn remove_program(&mut self, query_tuples: Vec<Tuple>, p: P) -> () {
        for Tuple(nouns) in query_tuples {
            let p = p.clone();
            let mut current_node_id: usize = 0;

            for n in nouns {
                match n {
                    TupleNoun::Prefix => {
                        // Write to prefix set and quit.
                        self.flat_node_tree.get_mut(current_node_id).unwrap().remove_from_prefix_programs(p);
                        return;
                    }

                    n => {
                        let next = self.get_children(current_node_id).get(&n);
                        match next {
                            Some(&next_id) => { current_node_id = next_id }
                            None => { break; }
                        }
                    }
                }
            }
            self.flat_node_tree.get_mut(current_node_id).unwrap().remove_from_exact_programs(p);
        }
    }

    fn query(&mut self, Tuple(nouns): &Tuple) -> Vec<P> {
        let mut output_programs: Vec<P> = Vec::new();

        let mut work_queue: VecDeque<(usize, &[TupleNoun])> = VecDeque::new();
        work_queue.push_back((0, &nouns));

        while let Some(np) = work_queue.pop_front() {
            // println!("Work Queue Popped: {np:?}");
            match np {
                (current_node_id, [n, nouns @ ..]) => {
                    // println!("Match: {current_node_id:?} n = {n:?}, nouns = {nouns:?}");
                    // 1. Add all prefix matchers at this point.
                    for p in self.flat_node_tree[current_node_id].matched_prefix_programs.iter() {
                        // println!("Adding prefix matches!");
                        output_programs.push(p.clone())
                    }

                    // 2. If there's a `Query` branch, add it to nodes-to-check
                    if let Some(&wildcard_id) = self.get_children(current_node_id).get(&TupleNoun::Query) {
                        // println!("Adding wildcard children!");
                        work_queue.push_back((wildcard_id, nouns))
                    }

                    // 3. Continue down the "exact" branch
                    if let Some(&exact_id) = self.get_children(current_node_id).get(n) {
                        // println!("Adding exact children!");
                        work_queue.push_back((exact_id, nouns))
                    }
                }

                (current_node_id, &[]) => {
                    for p in self.flat_node_tree[current_node_id].matched_exact_programs.iter() {
                        output_programs.push(p.clone())
                    }

                    for p in self.flat_node_tree[current_node_id].matched_prefix_programs.iter() {
                        output_programs.push(p.clone())
                    }
                }
            }
        }

        output_programs
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    pub fn paper_example_1() {
        // Q: <lexi, is a, husky>
        let mut engine = TripleQueryEngine::new();
        engine.insert_program_for_query(vec![Tuple::triple_from_strs(&["*", "is a", "husky"])], 1);

        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"]));
        assert_eq!(actual, vec![1]);
    }

    #[test]
    pub fn paper_example_2() {
        // Q: <lexi, is a, husky>
        let mut engine = TripleQueryEngine::new();
        engine.insert_program_for_query(vec![Tuple::triple_from_strs(&["lexi", "is a", "husky"])], 1);

        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"]));
        assert_eq!(actual, vec![1]);
    }

    #[test]
    pub fn paper_example_3() {
        // Q: <*, is a>
        let mut engine = TripleQueryEngine::new();
        engine.insert_program_for_query(vec![Tuple::triple_from_strs(&["*", "is a"])], 1);

        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"]));
        assert_eq!(actual, vec![]);

        let actual = engine.query(&Tuple::triple_from_strs(&["box", "is a"]));
        assert_eq!(actual, vec![1]);
    }

    #[test]
    pub fn paper_example_4() {
        // Q: <lexi, is a, husky>
        let mut engine = TripleQueryEngine::new();
        engine.insert_program_for_query(vec![Tuple::triple_from_strs(&["*", "is a", "..."])], 1);

        println!("{engine:?}");

        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"]));
        assert_eq!(actual, vec![1]);

        let actual = engine.query(&Tuple::triple_from_strs(&["box", "is a", "husky"]));
        assert_eq!(actual, vec![1]);

        let actual = engine.query(&Tuple::triple_from_strs(&["box", "is a"]));
        assert_eq!(actual, vec![1]);

        let actual = engine.query(&Tuple::triple_from_strs(&["box", "is a", "pretty", "husky"]));
        assert_eq!(actual, vec![1]);


        let actual = engine.query(&Tuple::triple_from_strs(&["box"]));
        assert_eq!(actual, vec![]);

        let actual = engine.query(&Tuple::triple_from_strs(&["box", "is"]));
        assert_eq!(actual, vec![]);
    }

    #[test]
    pub fn all_four_queries() {
        let mut engine = TripleQueryEngine::new();
        engine.insert_program_for_query(vec![Tuple::triple_from_strs(&["*", "is a", "husky"])], 1);
        engine.insert_program_for_query(vec![Tuple::triple_from_strs(&["lexi", "is a", "husky"])], 2);
        engine.insert_program_for_query(vec![Tuple::triple_from_strs(&["*", "is a"])], 3);
        engine.insert_program_for_query(vec![Tuple::triple_from_strs(&["*", "is a", "..."])], 4);

        println!("{engine:?}");

        assert_eq!(
            engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"])),
            vec![4, 1, 2]
        );
    }

    #[test]
    pub fn anything_about_lexi() {
        let mut engine = TripleQueryEngine::new();
        engine.insert_program_for_query(vec![Tuple::triple_from_strs(&["lexi", "..."])], 1);

        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"]));
        assert_eq!(actual, vec![1]);
    }


    #[test]
    pub fn removes_anything_about_lexi() {
        let mut engine = TripleQueryEngine::new();
        engine.insert_program_for_query(vec![Tuple::triple_from_strs(&["lexi", "..."])], 1);

        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"]));
        assert_eq!(actual, vec![1]);

        engine.remove_program(vec![Tuple::triple_from_strs(&["lexi", "..."])], 1);

        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"]));
        assert_eq!(actual, vec![]);
    }

    #[test]
    pub fn prefix_with_ors_work_1() {
        let mut engine = TripleQueryEngine::new();

        engine.insert_program_for_query(vec![
            Tuple::triple_from_strs(&["lexi", "..."]),
            Tuple::triple_from_strs(&["lexi is not", "..."]),
        ], 1);

        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"]));
        assert_eq!(actual, vec![1]);
        let actual = engine.query(&Tuple::triple_from_strs(&["lexi is not", "a", "husky"]));
        assert_eq!(actual, vec![1]);


    }
    #[test]
    pub fn prefix_with_ors_work_2() {
        let mut engine = TripleQueryEngine::new();

        engine.insert_program_for_query(vec![
            Tuple::triple_from_strs(&["lexi", "is a", "husky"]),
            Tuple::triple_from_strs(&["lexi is not", "..."]),
        ], 1);

        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"]));
        assert_eq!(actual, vec![1]);
        let actual = engine.query(&Tuple::triple_from_strs(&["lexi is not", "a", "husky"]));
        assert_eq!(actual, vec![1]);
        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is not a", "husky"]));
        assert_eq!(actual, vec![]);


    }
    #[test]
    pub fn prefix_with_ors_work_3() {
        let mut engine = TripleQueryEngine::new();
        engine.insert_program_for_query(vec![
            Tuple::triple_from_strs(&["lexi is not", "..."]),
            Tuple::triple_from_strs(&["lexi", "is a", "husky"]),
        ], 1);

        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is a", "husky"]));
        assert_eq!(actual, vec![1]);
        let actual = engine.query(&Tuple::triple_from_strs(&["lexi is not", "a", "husky"]));
        assert_eq!(actual, vec![1]);
        let actual = engine.query(&Tuple::triple_from_strs(&["lexi", "is not a", "husky"]));
        assert_eq!(actual, vec![]);

    }
    #[test]
    pub fn queries_match_a_or_b_not_c() {
        let mut engine = TripleQueryEngine::new();

        engine.insert_program_for_query(vec![
            Tuple::triple_from_strs(&["a"]),
            Tuple::triple_from_strs(&["b"])
        ], 1);

        let actual = engine.query(&Tuple::triple_from_strs(&["a"]));
        assert_eq!(actual, vec![1]);

        // let actual = engine.query(&Tuple::triple_from_strs(&["b"]));
        // assert_eq!(actual, vec![1]);

        let actual = engine.query(&Tuple::triple_from_strs(&["c"]));
        assert_eq!(actual, vec![]);
    }
}