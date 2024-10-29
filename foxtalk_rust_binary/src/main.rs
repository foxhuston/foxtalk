use crate::reactor_adding_so_handler::ReactorAddingSoHandler;
use dotenv;
use reactor::reactor::Reactor;
use reactor::triples_reactor::Tuple;
use std::sync::{Arc, Mutex};
use std::time::Instant;
use std::{fs, thread};
use crate::recursive_inotify::RecursiveFileWatcher;

mod cpp_handler_builder;
mod reactor_adding_so_handler;
mod recursive_inotify;
mod commands_json_creator;

fn main() {
    dotenv::dotenv().ok();

    let so_path_from_env = std::env::var("SO_PATH");
    let cpp_path_from_env = std::env::var("CPP_PATH");
    let handler_path_from_env = std::env::var("HANDLER_INCLUDE_PATH");

    if so_path_from_env.is_err() || cpp_path_from_env.is_err() {
        eprintln!("Error: SO_PATH and CPP_PATH and HANDLER_INCLUDE_PATH environment variable must be set.");
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

    let reactor: Arc<Mutex<Reactor<Tuple>>> = Arc::new(Mutex::new(Reactor::new()));
    let so_handler = ReactorAddingSoHandler::new(reactor.clone(), so_path.clone());
    let cpp_handler = cpp_handler_builder::CppFileBuilder {
        base_cpp_path: cpp_path.clone(),
        so_path: so_path.clone(),
        include_path: handler_path.clone(),
    };

    let cpp_watcher = Arc::new(RecursiveFileWatcher::new(cpp_path.clone(), Arc::new(cpp_handler)));
    RecursiveFileWatcher::watch(cpp_watcher);
    let so_watcher = Arc::new(RecursiveFileWatcher::new(so_path.clone(), Arc::new(so_handler)));
    RecursiveFileWatcher::watch(so_watcher);

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
        if tps.len() >= 10 {
            // println!("num ticks per second in the last 10 seconds: {:?}", tps);
            let ticks = tps.iter().sum::<u64>();
            let ticks_per_sec = ticks / tps.len() as u64;
            let k_ticks_per_sec = ticks_per_sec / 1000;
            let ticks_per_frame = ticks_per_sec / 120;
            println!("Avg ticks per sec: {:?}k || @120fps: {:?} ticks per frame || ticks: {:?}", k_ticks_per_sec, ticks_per_frame, ticks);
            println!("==vvvvv===Current counts===vvvvv==");
            println!("{:?}", reactor_guard.ref_counts);
            tps.clear();
        }
        thread::sleep(std::time::Duration::from_millis(4));
    }
}
