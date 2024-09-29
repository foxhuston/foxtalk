use std::path::PathBuf;
use std::process::Command;
use std::{env, fs};

fn main() {
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    // The bindgen::Builder is the main entry point
    // to bindgen, and lets you build up options for
    // the resulting bindings.
    let bindings = bindgen::Builder::default()
        // The input header we would like to generate
        // bindings for.
        .header("c/reactor_types.hpp")
        .allowlist_recursively(false)
        .allowlist_type("Tuple")
        .allowlist_type("TupleNoun")
        .derive_default(true)
        .derive_partialeq(true)
        .derive_eq(true)
        // Tell cargo to invalidate the built crate whenever any of the
        // included header files changed.
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");


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
                            "-I", manifest_dir.as_str(),
                            &filepath,
                            "-o", format!("tests/test_libs/out/{filename}.so"
                            ).as_str()])
                        .status();

                    assert_eq!(status.unwrap().success(), true);
                    println!("cargo::rerun-if-changed=tests/test_libs/src/{filename}.cpp");
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

