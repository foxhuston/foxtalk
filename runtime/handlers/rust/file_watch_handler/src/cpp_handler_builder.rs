use crate::commands_json_creator;
use crate::recursive_inotify::FileWatcherHandlers;
use std::fmt::{Debug, Formatter};
use std::fs;
use std::process::Command;

use log::{error, warn, info, debug, trace};

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
            
            let source = fs::read_to_string(full_path.clone()).unwrap();
            
            // if source starts with //foxtalk-link, then we need to add -l(whatever comes after // pkg-config)
            let link = source.split("\n").filter(|&c| {
                c.trim().starts_with("// pkg-config")
            }).map(|c| {
                c.replace("// pkg-config", "").trim().to_string()
            }).collect::<Vec<String>>();
            
            
            info!("Compiling {:?} to {:?}", full_path, output_so_file);

            fn pkg_config_args(arg: &str, l: &str) -> Vec<String> {
                if let Ok(pkg_libs_cmd) = Command::new("pkg-config")
                    .args([arg, l].iter())
                    .output() {
                    if pkg_libs_cmd.status.success() {
                        let pkg_libs = String::from_utf8(pkg_libs_cmd.stdout).unwrap();
                        pkg_libs.split_whitespace().collect()
                    } else {
                        error!("Failed to execute pkg-config {}", arg);
                        Vec::new()
                    }
                } else {
                    error!("Failed to execute pkg-config {}", arg);
                    Vec::new()
                }
            }

            let linking_args = link.iter().flat_map(|l| {
                vec![pkg_config_args("--libs", l), pkg_config_args("--cflags", l)].concat()
            }).collect::<Vec<String>>();

            let rest_of_args: Vec<String> = vec![
                "-shared",
                "-O0",
                "-std=c++26",
                "-I",
                self.include_path.as_str(),
                "-fPIC",
                "-MJ",
                output_json_file.as_str(),
                &full_path,
                "-o",
                output_so_file.as_str()
            ].iter().map(|&s| s.to_string()).collect();
            let status = Command::new("clang++")
                .args([linking_args, rest_of_args].concat())
                .status();

            if status.is_err() {
                error!("Error while compiling {:?}: {:?}", full_path, status);
            } else {
                info!("Compiled {:?} to {:?}", full_path, output_so_file);
            }
            // commands_json_creator::regenerate_compiler_commands();
        }
    }

    fn on_delete(&self, _: String, file_name: String, extension: String) -> () {
        if extension == "cpp" {
            let output_so_file = self.so_path.clone() + "/" + &file_name + ".so";
            let output_json_file = self.so_path.clone() + "/" + &file_name + ".json";
            // println!("Removing {:?} and {:?}", output_so_file, output_json_file);

            if let Err(err) = fs::remove_file(output_so_file.clone()) {
                error!("Error while removing {:?}: {:?}", output_so_file, err);
            }
            if let Err(err) = fs::remove_file(output_json_file.clone()) {
                error!("Error while removing {:?}: {:?}", output_json_file, err);
            }
        }
    }
}
