use crate::recursive_inotify::FileWatcherHandlers;
use reactor::reactor::Reactor;
use reactor::triples_reactor::{Tuple, TupleNoun};
use std::fmt::{Debug, Formatter};
use std::sync::{Arc, Mutex};
use walkdir::WalkDir;

pub struct ReactorAddingSoHandler {
    reactor: Arc<Mutex<Reactor<Tuple>>>
}

impl Debug for ReactorAddingSoHandler {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("SoHandler")
            .finish()
    }
}
impl FileWatcherHandlers for ReactorAddingSoHandler {
    fn on_create(&self, full_path: String, _: String, extension: String) -> () {
        println!("on_create {:?}", full_path);
        if extension == "so" {
            if let Some(tuple) = self.get_handler_tuple(full_path) {
                let mut reactor = self.reactor.lock().unwrap();
                reactor.remove(tuple.clone());
                reactor.insert(tuple.clone());
                println!("Added handler {:?}", tuple);
                println!("{:?}", reactor.ref_counts);
            }
        }
    }


    fn on_delete(&self, full_path: String, _: String, extension: String) -> () {
        println!("on_delete {:?}", full_path);
        if extension == "so" {
            if let Some(tuple) = self.get_handler_tuple(full_path) {
                let mut reactor = self.reactor.lock().unwrap();
                reactor.remove(tuple.clone());
                println!("Removed handler {:?}", tuple);
                println!("{:?}", reactor.ref_counts);
            }
        }
    }
}

impl ReactorAddingSoHandler {
    pub fn new(
        reactor: Arc<Mutex<Reactor<Tuple>>>,
        base_path: String) -> Self {
        let re = reactor.clone();
        let mut r = re.lock().unwrap();

        for entry in WalkDir::new(base_path.clone()).into_iter().filter_map(|e| e.ok()) {
            println!("{}", entry.path().display());

            if entry.file_name().to_str().unwrap().ends_with(".so") {
                if let Some(tuple) = Self::make_handler_tuple(Some(entry.path().to_str().unwrap().to_string())) {
                    r.remove(tuple.clone());
                    r.insert(tuple.clone());
                }
            }
        }
        ReactorAddingSoHandler { reactor }
    }
    fn make_handler_tuple(absolute_path: Option<String>) -> Option<Tuple> {
        absolute_path
            .map(|s| {
                if s.ends_with(".so") {
                    Some(Tuple(vec![TupleNoun::Symbol(s), TupleNoun::Symbol("is a".to_string()), TupleNoun::Symbol("handler".to_string())]))
                } else {
                    None
                }
            }).flatten()
    }

    fn get_handler_tuple(&self, full_path: String) -> Option<Tuple> {
        ReactorAddingSoHandler::make_handler_tuple(Some(full_path))
    }

}
