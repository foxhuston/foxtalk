use std::collections::{HashMap, VecDeque};
use std::sync::Arc;
use parking_lot::Mutex;
use anyhow::Result;
use inotify::{EventMask, Inotify, WatchDescriptor, WatchMask};
use log::info;
use rust_handler_macro::*;
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;


struct RecursiveFileWatchingHandler {
    last_seen_tuples: Vec<Tuple>,
    watches: HashMap<String, WatchDescriptor>,
    events: VecDeque<(String, EventMask)>,
    path_versions: HashMap<String, u64>,
    event_buffer: [u8; 1024 * 10],
    inotify: Inotify,
}

impl RecursiveFileWatchingHandler {
    fn watch(&mut self, path: &str) -> Result<WatchDescriptor> {
        if let Ok(wd) = self.inotify.watches()
            .add(path, WatchMask::CLOSE_WRITE | WatchMask::DELETE | WatchMask::CREATE | WatchMask::MOVED_TO | WatchMask::MOVED_FROM) {
            Ok(wd)
        } else { Err(anyhow::anyhow!("Failed to watch path {}", path)) }

    }

    fn output_tuples(&self) -> Vec<Tuple> {
        self.path_versions.iter().map(|(path, version)| {  //lol should just be the u64 but this is fine
            Tuple::triple_from_strs(&["file", path, "is", "at", "version", &version.to_string()])
        }).collect::<Vec<Tuple>>()
    }
}

impl NeedsToImplement for RecursiveFileWatchingHandler {
    fn query(&mut self) -> Vec<Tuple> {
        vec![Tuple::triple_from_strs(&["foxtalk", "handlers", "exist", "at", "absolute", "path", "*"])]
    }
    fn handle(&mut self, tuples: Vec<Tuple>) -> Vec<Tuple> {
        if self.last_seen_tuples != tuples {
            info!("There is a new watcher to create!");
            let all_paths: Vec<String> = tuples.iter().map(|tuple| {
                let Tuple(nouns) = tuple;
                match &nouns[..] {
                    [_, _, _, _, _, _, TupleNoun::Symbol(s)] => Some(s),
                    _ => None
                }
            }).filter_map(|x| x).cloned().collect();

            for path in all_paths.iter() {
                if self.watches.contains_key(path) {
                    continue;
                }
                if let Ok(wd) = self.watch(path) {
                    self.watches.insert(path.clone(), wd);
                } else {
                    error!("Failed to watch path {}", path);
                }
            }
            let mut remove = Vec::new();
            let wc = self.watches.clone();
            for (path, wd) in wc.iter() {
                if !all_paths.contains(path) {
                    remove.push(path);
                    if let Err(e) = self.inotify.watches().remove(wd.to_owned()) {
                        error!("Failed to remove watch for path {}: {}", path, e);
                    }
                }
            }
            for i in remove {
                self.watches.remove(i);
            }
            self.last_seen_tuples = tuples.clone();
        }

        for (file, mask) in self.events.drain(..) {
            if mask.contains(EventMask::DELETE | EventMask::MOVED_FROM | EventMask::DELETE_SELF) {
                self.path_versions.remove(&file);
            }
            if mask.contains(EventMask::CLOSE_WRITE | EventMask::MOVED_TO) {
                let version = self.path_versions.entry(file).or_insert(0);
                *version += 1;
            }
        }
        self.output_tuples()
    }

    fn poll(&mut self) -> bool {
        if let Ok(new_events) = self.inotify.read_events(self.event_buffer.as_mut()) {
            for event in new_events {
                if let Some(s_path) = event.name.map(|n| n.to_str()).flatten() {
                    let path = s_path.to_string();
                    self.events.push_back((path, event.mask));
                }
            }
        }

        !self.events.is_empty()
    }

    fn new() -> Self {
        RecursiveFileWatchingHandler {
            last_seen_tuples: vec![],
            watches: HashMap::new(),
            events: VecDeque::new(),
            path_versions: HashMap::new(),
            event_buffer: [0; 1024 * 10],
            inotify: Inotify::init().expect("Failed to start inotify"),
        }
    }
}

foxtalk_handler!(RecursiveFileWatchingHandler);

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn works() {
        let mut handler = RecursiveFileWatchingHandler::new();
        let input1 = vec![Tuple::triple_from_strs(&["foxtalk", "handlers", "exist", "at", "absolute", "path", "/tmp"])];
        let input2 = vec![
            Tuple::triple_from_strs(&["foxtalk", "handlers", "exist", "at", "absolute", "path", "/tmp"]),
            Tuple::triple_from_strs(&["foxtalk", "handlers", "exist", "at", "absolute", "path", "/tmp/foxtalk"]),
        ];
        let _ = handler.handle(input1.clone());
        assert_eq!(handler.watches.len(), 1);
        let _ = handler.handle(input1.clone());
        assert_eq!(handler.watches.len(), 1);
        let _ = handler.handle(input2.clone());
        assert_eq!(handler.watches.len(), 2);
        let _ = handler.handle(input2.clone());
        assert_eq!(handler.watches.len(), 2);
        let _ = handler.handle(input1.clone());
        assert_eq!(handler.watches.len(), 1);
        let _ = handler.handle(vec![]);
        assert_eq!(handler.watches.len(), 0);
    }
}