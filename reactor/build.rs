use std::path::PathBuf;
use std::process::Command;
use std::{env, fs};

fn main() {
    // This is needed so that dynamically linked libraries can find
    // `pub extern "C" fn`s from the main rust code.
    println!("cargo::rustc-link-arg=-export-dynamic");

    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let include_dir = PathBuf::from(&manifest_dir)
        .parent()
        .unwrap()
        .join("foxtalk_cpp_handler")
        .join("include");

    let dirs = fs::read_dir("tests/test_libs/src").unwrap();
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
                            "-g",
                            "-Ofast",
                            "-std=c++26",
                            "-I",
                            include_dir.as_os_str().to_str().unwrap(),
                            "-fPIC",
                            &filepath,
                            "-o",
                            format!("tests/test_libs/out/{filename}.so").as_str(),
                        ])
                        .status();

                    assert_eq!(status.unwrap().success(), true);
                    println!("cargo::rerun-if-changed=tests/test_libs/src/{filename}.cpp");
                    // println!("cargo::rerun-if-changed=tests/test_libs/out/{filename}.cpp");
                    println!("cargo::rerun-if-changed=../foxtalk_cpp_handler/include");
                }
            }
            _ => {}
        }
    }
}
