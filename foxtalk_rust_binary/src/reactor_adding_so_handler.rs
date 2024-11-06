use crate::recursive_inotify::FileWatcherHandlers;
use reactor::reactor::{Reactor, ReactorProgramId};
use reactor::triples_reactor::triple_query_engine::TripleQueryEngine;
use std::fmt::{Debug, Formatter};
use std::sync::{Arc, Mutex};
use rust_tuple_reactor_serde::tuple::Tuple;
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;
use walkdir::WalkDir;

use log::*;

pub struct ReactorAddingSoHandler {
    reactor: Arc<Mutex<Reactor<Vec<Tuple>, TripleQueryEngine<ReactorProgramId>, Tuple>>>,
}

impl Debug for ReactorAddingSoHandler {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("SoHandler").finish()
    }
}
impl FileWatcherHandlers for ReactorAddingSoHandler {
    fn on_create(&self, full_path: String, _: String, extension: String) -> () {
        trace!("on_create {:?}", full_path);
        if extension == "so" {
            if let Some(tuple) = self.get_handler_tuple(full_path) {
                debug!("Adding handler {:?}", tuple);
                let mut reactor = self.reactor.lock().unwrap();
                reactor.remove(tuple.clone());
                reactor.insert(tuple.clone());
                info!("Added handler {:?}", tuple);
                // println!("{:?}", reactor.ref_counts);
            }
        }
    }

    fn on_delete(&self, full_path: String, _: String, extension: String) -> () {
        trace!("on_delete {:?}", full_path);
        if extension == "so" {
            if let Some(tuple) = self.get_handler_tuple(full_path) {
                debug!("Removing handler {:?}", tuple);
                let mut reactor = self.reactor.lock().unwrap();
                reactor.remove(tuple.clone());
                info!("Removed handler {:?}", tuple);
                // println!("{:?}", reactor.ref_counts);
            }
        }
    }
}

impl ReactorAddingSoHandler {
    pub fn new(
        reactor: Arc<Mutex<Reactor<Vec<Tuple>, TripleQueryEngine<ReactorProgramId>, Tuple>>>,
        base_path: String,
    ) -> Self {
        let re = reactor.clone();
        let mut r = re.lock().unwrap();

        for entry in WalkDir::new(base_path.clone())
            .into_iter()
            .filter_map(|e| e.ok())
        {
            debug!("AddingSoHandler Scanning: {}", entry.path().display());

            if entry.file_name().to_str().unwrap().ends_with(".so") {
                if let Some(tuple) =
                    Self::make_handler_tuple(Some(entry.path().to_str().unwrap().to_string()))
                {
                    info!("AddingSoHandler Found: {}", entry.path().display());

                    r.remove(tuple.clone());
                    r.insert(tuple.clone());
                }
            }
        }

        let handler = ReactorAddingSoHandler { reactor };
        // for entry in WalkDir::new(base_path.clone())
        //     .into_iter()
        //     .filter_map(|e| e.ok())
        // {
        //     handler.on_create(entry.path().to_string_lossy().to_string(), "".to_string(), "so".to_string());
        // }


        handler
    }
    fn make_handler_tuple(absolute_path: Option<String>) -> Option<Tuple> {
        absolute_path
            .map(|s| {
                if s.ends_with(".so") {
                    Some(Tuple(vec![
                        TupleNoun::Symbol(s),
                        TupleNoun::Symbol("is a".to_string()),
                        TupleNoun::Symbol("handler".to_string()),
                    ]))
                } else {
                    None
                }
            })
            .flatten()
    }

    fn get_handler_tuple(&self, full_path: String) -> Option<Tuple> {
        ReactorAddingSoHandler::make_handler_tuple(Some(full_path))
    }
}
