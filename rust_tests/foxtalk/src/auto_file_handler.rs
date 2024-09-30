use std::sync::{Arc, Mutex};
use anyhow::{format_err, Result};
use reactor::ffi2::CWhen;
use reactor::tuple::Tuple;
use reactor::when::When;

pub struct AutoFileHandler {
    src_path: String,
    so_path: String,

    c_when: Arc<Mutex<Option<CWhen>>>,
}

impl AutoFileHandler {
    pub fn new(src_path: &str, so_path: &str) -> Result<Self> {
        let mut a = AutoFileHandler {
            src_path: src_path.to_string(),
            so_path: so_path.to_string(),

            c_when: Arc::new(Mutex::new(None))
        };

        a.rebuild()?;
        a.tryLoad()?;

        Ok(a)
    }

    fn rebuild(&self) -> Result<()> {
        let mut dir = std::env::current_dir()?;
        dir.push("reactor/c");
        let include_dir = dir.as_os_str().to_str().unwrap();
        println!("Adding include dir: {include_dir}");

        let out = std::process::Command::new("clang++")
            .args([
                // "-Wall", "-Wpedantic",
                "-std=c++26",
                "-shared",
                "-I", include_dir,
                "-fPIC",
                &self.src_path,
                "-o", &self.so_path])
            .output();

        match out {
            Ok(out) if out.status.success() => {
                let o = String::from_utf8(out.stdout)?;
                let e = String::from_utf8(out.stderr)?;
                println!("{}", o);
                println!("{}", e);
                Ok(())
            }
            Ok(out) => {
                let o = String::from_utf8(out.stdout)?;
                let e = String::from_utf8(out.stderr)?;
                println!("{}", o);
                println!("{}", e);

                Err(format_err!("AutoFileHandler compilation failed!"))
            }
            Err(err) => {
                Err(format_err!("{}", err))
            }
        }
    }

    fn tryLoad(&mut self) -> Result<()> {
        let mut c_when = self.c_when.lock().unwrap();

        match unsafe { CWhen::new(&self.so_path) } {
            Ok(cwhen) => {
                *c_when = Some(cwhen);
                Ok(())
            }
            Err(e) => { Err(e) }
        }
    }
}

impl When for AutoFileHandler {
    fn get_query(&self) -> Tuple {
        let src_path = &self.src_path;
        println!("AFH Query Begin [{src_path}]");
        let out = self.c_when.lock().unwrap().as_ref().unwrap().get_query();
        println!("AFH Query End [{src_path}]");
        out
    }

    fn handle(&mut self, results: Tuple) -> Vec<Tuple> {
        let src_path = &self.src_path;
        println!("AFH Handle Begin [{src_path}]");
        let out = self.c_when.lock().unwrap().as_mut().unwrap().handle(results);
        println!("AFH Handle End [{src_path}]");
        out
    }
}