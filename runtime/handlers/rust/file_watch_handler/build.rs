use std::env;
use std::fs;
use dotenv;
use std::path::Path;
fn main() {

    dotenv::dotenv().ok();
    println!("cargo:rerun-if-changed=src/lib.rs");

    let manifest_dir = env::var("CARGO_MANIFEST_DIR").expect("CARGO_MANIFEST_DIR not set");
    let profile = env::var("PROFILE").unwrap_or_else(|_| "debug".to_string());

    let manifest_path = Path::new(&manifest_dir);
    let workspace_target_dir = manifest_path
        .parent().unwrap()
        .parent().unwrap()
        .parent().unwrap()
        .join("target");

    // Define the source and destination paths

    let rust_so_path = env::var("RUST_SO_PATH").expect("RUST_SO_PATH not set");

    let source_path = Path::new(&workspace_target_dir).join(profile).join("libfile_watch_handler.so");
    let destination_path = Path::new(&rust_so_path).join("file_watch_handler.so");
    // println!("cargo:warning={}", source_path.display());

    fs::create_dir_all(Path::new(&rust_so_path)).expect("Failed to create RUST_SO_PATH");

    // Copy the cdylib to the desired location
    fs::copy(&source_path, &destination_path).expect("Failed to copy cdylib");

}