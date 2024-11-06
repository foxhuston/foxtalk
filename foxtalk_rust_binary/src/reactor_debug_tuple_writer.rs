use reactor::reactor::{Reactor, ReactorProgramId};
use reactor::triples_reactor::triple_query_engine::TripleQueryEngine;
use rust_tuple_reactor_serde::tuple::Tuple;
use std::sync::MutexGuard;
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;

pub struct ReactorDebugTupleWriter {
    last_tps_tuple: Option<Tuple>,
    last_ref_count_tuple: Option<Tuple>
}

impl ReactorDebugTupleWriter {
    pub fn new() -> Self {
        ReactorDebugTupleWriter {
            last_tps_tuple: None,
            last_ref_count_tuple: None,
        }
    }

    pub fn update_tps(&mut self,reactor_guard: &mut MutexGuard<Reactor<Vec<Tuple>, TripleQueryEngine<ReactorProgramId>, Tuple>>, tps: u64)  {
        if let Some(t) =  &self.last_tps_tuple  {
            reactor_guard.remove(t.clone());
        }
        let tuple = Tuple::triple_from_ssu(
            "foxtalk reactor",
            "is running at ticks per second of",
            tps,
        );
        reactor_guard.insert(tuple.clone());
        self.last_tps_tuple = Some(tuple);
    }
    pub fn update_reactor_tuples(&mut self, reactor_guard: &mut MutexGuard<Reactor<Vec<Tuple>, TripleQueryEngine<ReactorProgramId>, Tuple>>) {
        let string_repr = reactor_guard
            .ref_counts
            .iter()
            .filter(|(Tuple(nouns), _)| {
                let is_sees_tuples_tuple = nouns.len() >= 2 &&
                    nouns[0] == TupleNoun::Symbol("foxtalk reactor".to_string()) &&
                    nouns[1] == TupleNoun::Symbol("sees tuples".to_string());
                !is_sees_tuples_tuple
            })
            .fold(String::new(), |acc, (k, v)| {
                acc + &format!("{:?} [{:?}]\n", k, v)
            });

        let new_tuple_sees = Tuple::triple_from_sss(
            "foxtalk reactor",
            "sees tuples",
            string_repr.as_str(),
        );
        if let Some(t) =  &self.last_ref_count_tuple  {
                reactor_guard.remove(t.clone());
        }
        self.last_ref_count_tuple = Some(new_tuple_sees.clone());
        reactor_guard.insert(new_tuple_sees);


    }
}