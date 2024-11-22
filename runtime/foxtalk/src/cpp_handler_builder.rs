use crate::commands_json_creator;
use crate::recursive_inotify::FileWatcherHandlers;
use std::fmt::{Debug, Formatter};
use std::fs;
use std::process::Command;

use log::*;
use regex::Regex;

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

impl CppFileBuilder {

    fn find_cpp_std_in_source(source: String) -> String {
        let cppstd_regex = Regex::new(r"(//|/\*) *cppstd:? +(.*?)(\*/)?$").unwrap();


        let cppstd: Option<String> =
            source.split("\n")
                .filter_map(|line| {
                    let out: Option<&str> = cppstd_regex.captures(line)
                        .and_then(|c| c.get(2).map(|m| m.as_str()));

                    out
                }).find(|s| !s.is_empty())
                .map(|s| s.trim().to_string());

        cppstd.unwrap_or("26".to_string())
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

            let parent_path = full_path.rsplit_once("/").unwrap().0.to_string();
            let tupfile =  parent_path.clone() + "/Tupfile";
            if fs::metadata(tupfile.clone()).is_ok() {
                info!("Tupfile found next to {:?}, running tup instead of clang++", full_path);

                let tup_dir = parent_path.clone() + "/.tup";
                if fs::metadata(tup_dir).is_err() {
                    info!("First time needing to run tup for {:?}, running tup init", full_path);
                    let status = Command::new("tup")
                        .current_dir(parent_path.clone())
                        .arg("init")
                        .status();
                    if let Err(e) = status {
                        error!("Error while running tup init for {:?}: {:?}", full_path, e);
                        return;
                    }
                }
                let status = Command::new("tup")
                    .current_dir(parent_path.clone())
                    .status();
                if let Err(e) = status {
                    error!("Error while running tup for {:?}: {:?}", full_path, e);
                    return;
                }

                let tup_so_file = full_path.replace(".cpp", ".so");

                info!("Copying so file from tup: {:?} to the build location {:?}", tup_so_file, output_so_file);
                let status = Command::new("cp")
                    .arg(tup_so_file)
                    .arg(output_so_file.clone())
                    .status();
                if let Err(e) = status {
                    error!("Error while copying so file from tup for {:?}: {:?}", full_path, e);
                }

                return;
            }

            let output_json_file = output_so_path.clone() + ".json";

            let source = fs::read_to_string(full_path.clone()).unwrap();

            let config_regex = Regex::new(r"(//|/\*) *pkg-config:? +(.*?)(\*/)?$").unwrap();
            let sanitize_regex = Regex::new(r"(//|/\*) foxtalk: sanitize *$").unwrap();

            let lines: Vec<&str> = source.split("\n").collect();

            let packages: Vec<&str> = lines.iter()
                    .filter_map(|line| {
                        let out: Option<Vec<&str>> = config_regex.captures(line)
                            .and_then(|c| c.get(2)
                                .map(|m| m.as_str().split(" ")
                                    .filter(|m| !m.is_empty())
                                    .collect()));
                        out
                    })
                    .flatten()
                    .collect();

            debug!("In cpp handler on_create (for close_write or moved_to): Found {:?} in {:?}", packages, full_path);

            let should_sanitize = lines.iter().any(|line| sanitize_regex.is_match(line));

            fn pkg_config_args(arg: &str, package_names: &[&str]) -> Vec<String> {
                if let Ok(pkg_libs_cmd) = Command::new("pkg-config")
                    .args([arg].iter().chain(package_names))
                    .output() {
                    if pkg_libs_cmd.status.success() {
                        let pkg_libs = String::from_utf8(pkg_libs_cmd.stdout).unwrap_or("".to_string());
                        pkg_libs.split_whitespace().map(|s| s.trim().to_string()).collect()
                    } else {
                        error!("Failed to execute pkg-config {}", arg);
                        Vec::new()
                    }
                } else {
                    error!("Failed to execute pkg-config {}", arg);
                    Vec::new()
                }
            }

            let linking_args: Vec<String> =
                if packages.len() > 0 {
                    [pkg_config_args("--libs", &packages), pkg_config_args("--cflags", &packages)]
                        .concat()
                } else {
                    Vec::new()
                };

            info!("In cpp handler on_create (for close_write or moved_to): Compiling {:?} to {:?}", full_path, output_so_file);

            let cpp_standard = CppFileBuilder::find_cpp_std_in_source(source);
            let cpp_std_str = format!("-std=c++{}", cpp_standard);

            let sanitize_arg = if should_sanitize {
                vec!["-fsanitize=address".to_string()]
            } else {
                vec![]
            };

            let rest_of_args: Vec<String> = vec![
              "-shared",
              "-O0",
              "-g",
              cpp_std_str.as_str(),
              "-I",
              self.include_path.as_str(),
              "-fPIC",
              "-MJ",
              output_json_file.as_str(),
              &full_path,
              "-o",
              output_so_file.as_str()
            ].iter().map(|&s| s.to_string()).collect();
            let all_args = [sanitize_arg, rest_of_args, linking_args].concat();
            trace!("Running command clang++ {:?}", all_args);

            let status = Command::new("clang++")
                .args(all_args)
                .status();

            match status {
                Ok(s) if !s.success() => {
                  error!("Error while compiling {:?}: {:?}", full_path, s);
                  warn!("code is {:?}", s.code());
                }
                Ok(_) => {
                  info!("Compiled {:?} to {:?}", full_path, output_so_file);
                  commands_json_creator::regenerate_compiler_commands();
                }
                Err(e) if e.raw_os_error() == Some(2) => {
                  error!("Error while compiling {:?}: {:?}", full_path, e);
                  error!(">>>>>> Is clang++ installed? <<<<<<");
                }
                Err(e) => {
                  error!("Error while compiling {:?}: {:?}", full_path, e);
                }
            }
        }
    }

    fn on_delete(&self, _: String, file_name: String, extension: String) -> () {
        if extension == "cpp" {
            let output_so_file = self.so_path.clone() + "/" + &file_name + ".so";
            let output_json_file = self.so_path.clone() + "/" + &file_name + ".json";
            debug!("From cpp handler on_delete (or moved_from): Removing {:?} and {:?}", output_so_file, output_json_file);

            if let Err(err) = fs::remove_file(output_so_file.clone()) {
                error!("Error while removing {:?}: {:?}", output_so_file, err);
            }
            if let Err(err) = fs::remove_file(output_json_file.clone()) {
                error!("Error while removing {:?}: {:?}", output_json_file, err);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;


    #[test]
    fn it_should_get_the_cpp_standard() {
        let source = "// pkg-config: opencv4\n// cppstd: 23\n\n#include <foxtalk_handler.hpp>\n#include <opencv2/opencv.hpp>\n#include <ostream>";
        let cppstd = CppFileBuilder::find_cpp_std_in_source(source.to_string());
        assert_eq!(cppstd, "23".to_string());
    }
    #[test]
    fn it_should_default_to_cpp_standard_26() {
        let source = "// pkg-config: opencv4\n\n#include <foxtalk_handler.hpp>\n#include <opencv2/opencv.hpp>\n#include <ostream>";
        let cppstd = CppFileBuilder::find_cpp_std_in_source(source.to_string());
        assert_eq!(cppstd, "26".to_string());
    }
}