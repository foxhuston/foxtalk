use tokio::sync::Mutex;
use std::sync::Arc;

use reactor::ffi2::CWhen;

pub struct AutoFileHandler {
    src_path: String,
    so_path: String,

    c_when: Arc<Mutex<Option<CWhen>>>,
}

impl AutoFileHandler {
    pub async fn new(src_path: &str, so_path: &str) -> Self {
        let mut a = AutoFileHandler {
            src_path: src_path.to_string(),
            so_path: so_path.to_string(),

            c_when: Arc::new(Mutex::new(None))
        };

        a.rebuild();
        a.tryLoad().await;

        a
    }

    fn rebuild(&self) {
        let out = std::process::Command::new("clang++")
            .args(["-Wall", "-Wpedantic", "-shared", &self.src_path, "-o", &self.so_path])
            .output()
            .expect("Failed to rebuild {&self.src_path}");


        let o = String::from_utf8(out.stdout).unwrap();
        let e = String::from_utf8(out.stderr).unwrap();

        println!("{}", o);
        println!("{}", e);
    }

    async fn tryLoad(&mut self) {
        let mut c_when = self.c_when.lock().await;
        match unsafe { CWhen::new(&self.so_path) } {
            Ok(cwhen) => { *c_when = Some(cwhen) }
            Err(e) => {
                println!("{}", e);
            }
        }
    }
}