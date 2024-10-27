use inotify::{Event, EventMask, Inotify, WatchMask};
use reactor::reactor::Reactor;
use reactor::triples_reactor::{Tuple, TupleNoun};
use std::ffi::OsStr;
use std::sync::{Arc, Mutex};
use std::time::Instant;
use std::{fs, io, thread};
use dotenv;

struct FileWatchHandler{
    reactor: Arc<Mutex<Reactor<Tuple>>>,
    base_path: String,
}

impl FileWatchHandler {
    
    fn new(
        reactor: Arc<Mutex<Reactor<Tuple>>>,
        base_path: String) -> Self {
        
        let re = reactor.clone();
        let mut r = re.lock().unwrap();
        let dirs = fs::read_dir(base_path.clone()).unwrap();
        for file_entry in dirs {
            if let Ok(file) = file_entry {
                if let Some(tuple) = Self::make_handler_tuple(Some(file.path().to_str().unwrap().to_string())) {
                    r.remove(tuple.clone());
                    r.insert(tuple);
                }

            }
        }


        FileWatchHandler { reactor, base_path }
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

    fn get_handler_tuple(&self, file_name: &OsStr) -> Option<Tuple> {
        let absolute_path = self.base_path.clone() + "/" + file_name.to_str().unwrap();
        FileWatchHandler::make_handler_tuple(Some(absolute_path))
    }

    fn handle_event(&mut self, event: Event<&OsStr>) {

        match event {
            Event {mask: EventMask::CLOSE_WRITE, name: Some(file_name), ..} => {
                if let Some(tuple) = self.get_handler_tuple(file_name) {
                    let mut reactor = self.reactor.lock().unwrap();
                    reactor.remove(tuple.clone());
                    reactor.insert(tuple);
                }
            },
            Event {mask: EventMask::DELETE, name: Some(file_name), ..} => {
                if let Some(tuple) = self.get_handler_tuple(file_name) {
                    let mut reactor = self.reactor.lock().unwrap();
                    reactor.remove(tuple);
                }
            },
            _ => println!("Unhandled event {:?}", event),
        }
    }
}

fn main() {
    dotenv::dotenv().ok();

    let path_from_env = std::env::var("WATCH_PATH");
    let path = path_from_env.unwrap_or_else(|_| {
        println!("Info: No WATCH_PATH environment variable set. What absolute path do you want to watch?");
        let mut input = String::new();
        match io::stdin().read_line(&mut input) {
            Ok(_) => { input.trim().to_string() },
            Err(_no_updates_is_fine) => {
                eprintln!("Error reading input. Exiting.");
                std::process::exit(1);
            },
        }
    });

    let reactor: Arc<Mutex<Reactor<Tuple>>> = Arc::new(Mutex::new(Reactor::new()));

    let mut handler = FileWatchHandler::new(reactor.clone(), path.clone());
    
    println!("Watching for changes in {:?}", path);

    thread::spawn(move || {
        let mut inotify = Inotify::init()
            .expect("Error while initializing inotify instance");
        inotify
            .watches()
            .add(
                path,
                WatchMask::CLOSE_WRITE | WatchMask::DELETE,
            )
            .expect("Failed to add file watch");

        let mut buffer = [0; 1024];
        loop {
            let events = inotify.read_events_blocking(&mut buffer)
                .expect("Error while reading events");

            for event in events {
                handler.handle_event(event);
            }
        }
    });

    let mut cnt = 0;
    let mut current_time = Instant::now();
    let mut tps = Vec::new();

    println!("Starting reactor...");
    loop {
        cnt += 1;
        let mut reactor_guard = reactor.lock().unwrap();
        reactor_guard.tick();
        let new_time = Instant::now();
        if new_time.duration_since(current_time).as_secs() >= 1 {
            tps.push(cnt);
            cnt = 0;
            current_time = new_time;
        }
        if tps.len() >= 1 {
            // println!("num ticks per second in the last 10 seconds: {:?}", tps);
            let ticks_per_sec = tps.iter().sum::<u64>() / tps.len() as u64;
            let k_ticks_per_sec = ticks_per_sec / 1000;
            let ticks_per_frame = ticks_per_sec / 120;
            println!("Avg ticks per sec: {:?}k || @120fps: {:?} ticks per frame", k_ticks_per_sec, ticks_per_frame);
            tps.clear();
        }
    }
}
