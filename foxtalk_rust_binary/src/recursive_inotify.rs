use inotify::{EventMask, Inotify, WatchMask};
use std::collections::HashMap;
use std::fmt::Debug;
use std::sync::{Arc, RwLock};
use std::thread;

pub trait FileWatcherHandlers
where
    Self: Send + Sync + Debug,
{
    fn on_create(&self, full_path: String, file_name: String, extension: String) -> ();
    fn on_delete(&self, full_path: String, file_name: String, extension: String) -> ();
}

pub struct RecursiveFileWatcher {
    base_path: String,
    child_watch_descriptors: Arc<RwLock<HashMap<String, Arc<RecursiveFileWatcher>>>>,
    user_handlers: Arc<dyn FileWatcherHandlers>,
}

impl RecursiveFileWatcher {
    pub fn new(base_path: String, h: Arc<dyn FileWatcherHandlers>) -> Self {
        RecursiveFileWatcher {
            base_path,
            child_watch_descriptors: Arc::new(RwLock::new(HashMap::new())),
            user_handlers: h,
        }
    }

    pub fn watch(watcher: Arc<RecursiveFileWatcher>) {
        // println!("Now watching {:?}", watcher.base_path.clone());
        thread::spawn(move || {
            let mut inotify = Inotify::init()
                .expect("Error while initializing inotify instance for cpp instance");
            inotify
                .watches()
                .add(
                    watcher.base_path.clone(),
                    WatchMask::CLOSE_WRITE | WatchMask::DELETE | WatchMask::CREATE,
                )
                .expect("Failed to add file watch");
            let sub_directories = std::fs::read_dir(watcher.base_path.clone()).unwrap();
            for entry in sub_directories {
                let entry = entry.unwrap();
                if entry.file_type().unwrap().is_dir() {
                    let new_base_path = entry.path().to_str().unwrap().to_string();

                    // println!("Creating CPP watch for {:?}", new_base_path);
                    let new_watcher = Arc::new(RecursiveFileWatcher {
                        base_path: new_base_path,
                        child_watch_descriptors: Arc::new(RwLock::new(HashMap::new())),
                        user_handlers: watcher.user_handlers.clone(),
                    });
                    watcher
                        .child_watch_descriptors
                        .write()
                        .unwrap()
                        .insert(new_watcher.base_path.clone(), new_watcher.clone());
                    RecursiveFileWatcher::watch(new_watcher);
                }
            }

            let mut buffer = [0; 1024];
            loop {
                let events = inotify
                    .read_events_blocking(&mut buffer)
                    .expect("Error while reading events");

                for event in events {
                    // println!("{:?}", event);

                    let base_path = watcher.base_path.clone() + "/";
                    if event.name.is_none() {
                        eprintln!("Error for event.name: {:?}", event.name);
                        continue;
                    }
                    let full_path_to_file =
                        base_path.clone() + event.name.unwrap().to_str().unwrap();
                    if event.mask == EventMask::CREATE | EventMask::ISDIR {
                        // println!("Creating watch for {:?}", full_path_to_file);
                        let new_watcher = Arc::new(RecursiveFileWatcher {
                            base_path: full_path_to_file.clone(),
                            child_watch_descriptors: Arc::new(RwLock::new(HashMap::new())),
                            user_handlers: watcher.user_handlers.clone(),
                        });
                        RecursiveFileWatcher::watch(new_watcher.clone());
                        watcher
                            .child_watch_descriptors
                            .write()
                            .unwrap()
                            .insert(full_path_to_file.clone(), new_watcher);
                        continue;
                    }
                    if event.mask == EventMask::DELETE | EventMask::ISDIR {
                        // println!("Removing watch for {:?} and all subdirectories", full_path_to_file);
                        let wd_arc = watcher.child_watch_descriptors.clone();
                        let mut wd = wd_arc.write().unwrap();
                        wd.remove(&full_path_to_file);
                        continue;
                    }

                    let file_path = event.name.unwrap().to_str().unwrap().to_string();
                    let (fin, ext) = file_path.rsplit_once(".").unwrap();
                    let file_name = fin.to_string();
                    let extension = ext.to_string();

                    if event.mask == EventMask::DELETE {
                        // println!("Deleting file {:?}", full_path_to_file);
                        watcher.user_handlers.on_delete(
                            full_path_to_file.clone(),
                            file_name.clone(),
                            extension.clone(),
                        );
                        continue;
                    }
                    if event.mask == EventMask::CLOSE_WRITE {
                        // println!("Writing to file {:?}", full_path_to_file);
                        watcher.user_handlers.on_create(
                            full_path_to_file.clone(),
                            file_name.clone(),
                            extension.clone(),
                        );
                        continue;
                    }
                    if event.mask == EventMask::CREATE {
                        continue;
                    }

                    println!(
                        "Waning: Found unhandled event {:?} for {:?}.",
                        event.mask, full_path_to_file
                    );
                }
            }
        });
    }
}
