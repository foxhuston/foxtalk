use crate::commands_json_creator;
use crate::recursive_inotify::FileWatcherHandlers;
use std::fmt::{Debug, Formatter};
use std::fs;
use std::process::Command;

pub struct CppFileBuilder {
    pub base_cpp_path: String,
    pub so_path: String,
    pub include_path: String,
}

impl Debug for CppFileBuilder {
    fn fmt(&self, f: &mut Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("CppFileBuilder")
            .field("base_cpp_path", &self.base_cpp_path)
            .field("so_path", &self.so_path)
            .field("include_path", &self.include_path)
            .finish()
    }
}

impl FileWatcherHandlers for CppFileBuilder {
    fn on_create(&self, full_path: String, _: String, extension: String) -> () {
        if extension == "cpp" {
            let output_so_path = full_path
                .replace(&self.base_cpp_path, &self.so_path)
                .rsplit_once(".")
                .unwrap()
                .0
                .to_string();
            let parent_so_path = output_so_path.rsplit_once("/").unwrap().0.to_string();
            fs::create_dir_all(parent_so_path.clone()).unwrap();
            let output_so_file = output_so_path.clone() + ".so";
            let output_json_file = output_so_path.clone() + ".json";
            // println!("Compiling {:?} to {:?}", full_path, output_so_file);
            let status = Command::new("clang++")
                .args([
                    "-shared",
                    // "-g",
                    "-Ofast",
                    "-std=c++26",
                    "-I",
                    self.include_path.as_str(),
                    "-fPIC",
                    "-MJ",
                    output_json_file.as_str(),
                    &full_path,
                    "-o",
                    output_so_file.as_str(),
                ])
                .status();

            if status.is_err() {
                eprintln!("Error while compiling {:?}: {:?}", full_path, status);
            } else {
                // println!("Compiled {:?} to {:?}", full_path, output_so_file);
            }
            commands_json_creator::regenerate_compiler_commands();
        }
    }

    fn on_delete(&self, _: String, file_name: String, extension: String) -> () {
        if extension == "cpp" {
            let output_so_file = self.so_path.clone() + "/" + &file_name + ".so";
            let output_json_file = self.so_path.clone() + "/" + &file_name + ".json";
            // println!("Removing {:?} and {:?}", output_so_file, output_json_file);

            if let Err(err) = fs::remove_file(output_so_file.clone()) {
                eprintln!("Error while removing {:?}: {:?}", output_so_file, err);
            }
            if let Err(err) = fs::remove_file(output_json_file.clone()) {
                eprintln!("Error while removing {:?}: {:?}", output_json_file, err);
            }
        }
    }
}
