use std::path::PathBuf;
use std::process::Command;
use std::{env, fs};

use cbindgen;

fn main() {
    // This is needed so that dynamically linked libraries can find
    // `pub extern "C" fn`s from the main rust code.
    println!("cargo::rustc-link-arg=-export-dynamic");

    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let include_dir = PathBuf::from(&manifest_dir).join("c");

    // let walker = WalkDir::new("tests/test_libs/src").into_iter();
    let dirs =  fs::read_dir("tests/test_libs/src").unwrap();
    fs::create_dir_all("tests/test_libs/out").unwrap();
    for e in dirs {
        match e {
            Ok(entry) if entry.file_type().unwrap().is_file() => {
                let filename = entry.file_name();
                let filename = filename.to_string_lossy();
                let filename = filename.split('.').next().unwrap();

                let filepath = entry.path();
                let filepath = filepath.to_str().unwrap();

                if filepath.ends_with(".cpp") {
                    let status = Command::new("clang++")
                        .args([
                            "-shared",
                            "-I", include_dir.as_os_str().to_str().unwrap(),
                            &filepath,
                            "-o", format!("tests/test_libs/out/{filename}.so"
                            ).as_str()])
                        .status();

                    assert_eq!(status.unwrap().success(), true);
                    // println!("cargo::rerun-if-changed=tests/test_libs/src/{filename}.cpp");
                }
            }
            _ => {}
        }
    }

    // for e in walker {
    //     if e.is_err() {
    //         continue;
    //     }
    //     if e.as_ref().unwrap().path().is_dir() {
    //         continue;
    //     }
    //
    //     let entry = e.unwrap();
    //     let filename = entry
    //         .file_name()
    //         .to_str()
    //         .unwrap_or("unknown")
    //         .split(".")
    //         .next()
    //         .unwrap();
    //
    //     let status = Command::new("clang++")
    //         .args([
    //             "-shared",
    //             "-I", manifest_dir.as_str(),
    //             entry.path().as_os_str().to_str().unwrap(),
    //             "-o", format!("tests/test_libs/out/{filename}.so"
    //             ).as_str()])
    //         .status();
    //
    //     assert_eq!(status.unwrap().success(), true);
    //     println!("cargo::rerun-if-changed=tests/test_libs/src/{filename}.cpp");
    // }
}

