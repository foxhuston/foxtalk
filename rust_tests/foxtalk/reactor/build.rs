use std::process::Command;
use walkdir::WalkDir;

fn main() {

    let walker = WalkDir::new("tests/test_libs/src").into_iter();
    for e in walker {
        if e.is_err() {
            continue;
        }
        if e.as_ref().unwrap().path().is_dir() {
            continue;
        }

        let entry = e.unwrap();
        let filename = entry
            .file_name()
            .to_str()
            .unwrap_or("unknown")
            .split(".")
            .next()
            .unwrap();

        let status = Command::new("clang++")
            .args([
                // "-stdlib=libc++",
                "-shared",
                entry.path().as_os_str().to_str().unwrap(),
                "-o", format!("tests/test_libs/out/{filename}.so").as_str(),
                // "-std=c++23",
                // "-fmodules"
            ])
            .status();

        assert_eq!(status.unwrap().success(), true);
        println!("cargo::rerun-if-changed=tests/test_libs/src/{filename}.cpp");
    }
}

