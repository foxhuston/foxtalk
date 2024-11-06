use crate::reactor_adding_so_handler::ReactorAddingSoHandler;
use crate::recursive_inotify::RecursiveFileWatcher;
use dotenv;
use reactor::reactor::{Reactor, ReactorProgramId};
use reactor::triples_reactor::triple_query_engine::TripleQueryEngine;
use rust_tuple_reactor_serde::tuple::Tuple;
use std::sync::{Arc, Mutex};
use std::time::Instant;
use std::{fs, thread};
use crate::reactor_debug_tuple_writer::ReactorDebugTupleWriter;

use log::{error, warn, info, debug, trace, LevelFilter};
use regex::Regex;

mod commands_json_creator;
mod cpp_handler_builder;
mod reactor_adding_so_handler;
mod recursive_inotify;
mod reactor_debug_tuple_writer;

fn main() {
    colog::default_builder().filter_level(LevelFilter::Trace).init();
    dotenv::dotenv().ok();

    let so_path_from_env = std::env::var("SO_PATH");
    let cpp_path_from_env = std::env::var("CPP_PATH");
    let handler_path_from_env = std::env::var("HANDLER_INCLUDE_PATH");

    if so_path_from_env.is_err() || cpp_path_from_env.is_err() {
        error!("Error: SO_PATH and CPP_PATH and HANDLER_INCLUDE_PATH environment variable must be set.");
        std::process::exit(1);
    }

    let so_path = so_path_from_env.unwrap();
    let cpp_path = cpp_path_from_env.unwrap();
    let handler_path = handler_path_from_env.unwrap();

    if !fs::exists(so_path.clone()).unwrap() {
        fs::create_dir_all(so_path.clone()).unwrap();
    }

    if !fs::exists(cpp_path.clone()).unwrap() {
        fs::create_dir_all(cpp_path.clone()).unwrap();
    }

    let query_engine = TripleQueryEngine::new();
    let reactor: Arc<Mutex<Reactor<Vec<Tuple>, TripleQueryEngine<ReactorProgramId>, Tuple>>> =
        Arc::new(Mutex::new(Reactor::new(query_engine)));
    let so_handler = ReactorAddingSoHandler::new(reactor.clone(), so_path.clone());
    let cpp_handler = cpp_handler_builder::CppFileBuilder {
        base_cpp_path: cpp_path.clone(),
        so_path: so_path.clone(),
        include_path: handler_path.clone(),
    };

    let cpp_watcher = Arc::new(RecursiveFileWatcher::new(
        cpp_path.clone(),
        Arc::new(cpp_handler),
    ));
    RecursiveFileWatcher::watch(cpp_watcher);
    let so_watcher = Arc::new(RecursiveFileWatcher::new(
        so_path.clone(),
        Arc::new(so_handler),
    ));
    RecursiveFileWatcher::watch(so_watcher);

    {
        let mut reactor_guard = reactor.lock().unwrap();
        reactor_guard.insert(Tuple::triple_from_sss(
            "foxtalk",
            "is",
            "running",
        ));
    }

    let mut cnt = 0;
    let mut current_time = Instant::now();
    let mut tps = Vec::new();

    info!("Starting reactor...");
    let mut tuple_writer = ReactorDebugTupleWriter::new();

    loop {
        cnt += 1;
        {
            let mut reactor_guard = reactor.lock().unwrap();
            reactor_guard.tick();
            let new_time = Instant::now();
            if new_time.duration_since(current_time).as_secs() >= 1 {
                tps.push(cnt);
                cnt = 0;
                current_time = new_time;
            }
            if tps.len() >= 1 {
                let ticks = tps.iter().sum::<u64>();
                let ticks_per_sec = ticks / tps.len() as u64;
                tuple_writer.update_reactor_tuples(&mut reactor_guard);
                tuple_writer.update_tps(&mut reactor_guard, ticks_per_sec);
                tps.clear();
            }
        }
        // thread::sleep(std::time::Duration::from_micros(16666));
        thread::sleep(std::time::Duration::from_micros(100));
    }
}
