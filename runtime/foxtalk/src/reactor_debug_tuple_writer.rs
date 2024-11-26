use reactor::reactor::{Reactor, ReactorProgramId};
use reactor::triples_reactor::triple_query_engine::TripleQueryEngine;
use rust_tuple_reactor_serde::tuple::Tuple;
use std::sync::MutexGuard;
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;

pub struct ReactorDebugTupleWriter {
    so_path: String,
    last_tps_tuple: Option<Tuple>,
    last_ref_count_tuple: Option<Tuple>,
    last_handler_tuple: Option<Tuple>,
    last_debug_tuple: Option<Tuple>,
    last_error_tuple: Option<Tuple>,
}

impl ReactorDebugTupleWriter {
    pub fn new(so_path: String) -> Self {
        ReactorDebugTupleWriter {
            so_path,
            last_tps_tuple: None,
            last_ref_count_tuple: None,
            last_handler_tuple: None,
            last_debug_tuple: None,
            last_error_tuple: None,
        }
    }

    pub fn update_tps(&mut self, reactor_guard: &mut MutexGuard<Reactor<Vec<Tuple>, TripleQueryEngine<ReactorProgramId>, Tuple>>, tps: u64) {
        if let Some(t) = &self.last_tps_tuple {
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
        let mut objects: Vec<String> = vec![];
        let mut handlers: Vec<String> = vec![];
        let mut debug_messages: Vec<(String, f64)> = vec![];
        let mut error_messages: Vec<(String, f64)> = vec![];


        for (k, v) in reactor_guard
            .ref_counts
            .iter()
            .filter(|(Tuple(nouns), _)| {
                let is_recursive = nouns.len() >= 2 &&
                    nouns[0] == TupleNoun::Symbol("foxtalk reactor".to_string()) &&
                    nouns[1] == TupleNoun::Symbol("has messages".to_string());
                !is_recursive
            })
        {
            let Tuple(nouns) = k;
            let default = TupleNoun::Symbol("unknown".to_string());
            let name = nouns.first().unwrap_or(&default).as_str();
            let second = nouns.get(1).unwrap_or(&default);
            if nouns.contains(&TupleNoun::Symbol("handler".to_string())) {
                let so_name = name.replace(self.so_path.as_str(), "");
                handlers.push(format!("{}", so_name));
            } else if second == &TupleNoun::Symbol("has debug message".to_string()) && nouns.len() > 3 {
                let msg = nouns[2].as_str();
                if let TupleNoun::Double(c) = nouns[3] {
                    debug_messages.push((format!("[{}] {} [{}]", name, msg, c), c));
                } else {
                    objects.push(format!("{:?} [{:?}]", k, v));
                }
            } else if second == &TupleNoun::Symbol("has error message".to_string()) && nouns.len() > 3{
                let msg = nouns[2].as_str();
                if let TupleNoun::Double(c) = nouns[3] {
                    error_messages.push((format!("[{}] {} [{}]", name, msg, c), c));
                } else {
                    objects.push(format!("{:?} [{:?}]", k, v));
                }
            } else {
                objects.push(format!("{:?} [{:?}]", k, v));
            }
        }

        objects.sort();
        let obj_string_repr = objects.join("\n");

        let new_obj_tuple = Tuple::triple_from_ssss(
            "foxtalk reactor",
            "has messages",
            "listing the object db",
            obj_string_repr.as_str(),
        );
        handlers.sort();
        let handler_string_repr = handlers.join("\n");

        let new_handler_tuple = Tuple::triple_from_ssss(
            "foxtalk reactor",
            "has messages",
            "listing all handlers",
            handler_string_repr.as_str(),
        );
        debug_messages.sort_by(|a, b| a.1.partial_cmp(&b.1).unwrap());
        let debug_msgs: Vec<String> = debug_messages.iter().map(|(a, _)| { a.clone() }).collect();
        let debug_string_repr = debug_msgs.join("\n");

        let new_debug_tuple = Tuple::triple_from_ssss(
            "foxtalk reactor",
            "has messages",
            "with handler debug messages",
            debug_string_repr.as_str(),
        );
        error_messages.sort_by(|a, b| a.1.partial_cmp(&b.1).unwrap());
        let err_msgs: Vec<String> = error_messages.iter().map(|(a, _)| { a.clone() }).collect();
        let error_string_repr = err_msgs.join("\n");

        let new_error_tuple = Tuple::triple_from_ssss(
            "foxtalk reactor",
            "has messages",
            "with handler error messages",
            error_string_repr.as_str(),
        );
        if let Some(t) = &self.last_ref_count_tuple {
            reactor_guard.remove(t.clone());
        }
        if let Some(t) = &self.last_error_tuple {
            reactor_guard.remove(t.clone());
        }
        if let Some(t) = &self.last_debug_tuple {
            reactor_guard.remove(t.clone());
        }
        if let Some(t) = &self.last_handler_tuple {
            reactor_guard.remove(t.clone());
        }
        self.last_ref_count_tuple = Some(new_obj_tuple.clone());
        reactor_guard.insert(new_obj_tuple);
        self.last_handler_tuple = Some(new_handler_tuple.clone());
        reactor_guard.insert(new_handler_tuple);
        self.last_debug_tuple = Some(new_debug_tuple.clone());
        reactor_guard.insert(new_debug_tuple);
        self.last_error_tuple = Some(new_error_tuple.clone());
        reactor_guard.insert(new_error_tuple);
    }
}