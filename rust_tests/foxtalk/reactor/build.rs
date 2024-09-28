use std::process::Command;
use walkdir::WalkDir;

fn main() {
    let walker = WalkDir::new("tests/test_libs/src").into_iter();
    for e in walker {

        if e.is_err() {
            // println!("error");
            // println!("{:?}", e);
            continue;
        }
        if e.as_ref().unwrap().path().is_dir() {
            // println!("is directory");
            // println!("{:?}", e);
            continue;
        }

        println!("{:?}", e);
        let entry = e.unwrap();
        println!("{:?}", entry);
        let filename = entry
            .file_name()
            .to_str()
            .unwrap_or("unknown")
            .split(".")
            .next()
            .unwrap();

        let status = Command::new("clang++")
            .args(["-shared", entry.path().as_os_str().to_str().unwrap(), "-o", format!("tests/test_libs/out/{filename}.so").as_str()])
            .status();

        assert_eq!(status.unwrap().success(), true);
        // cc::Build::new()
        //     .cpp(true)
        //     .compiler("clang++")
        //     .no_default_flags(true)
        //     .file(entry.path())
        //     .shared_flag(true)
        //     .out_dir("tests/test_libs/out")
        //     .compile(format!("{filename}.so").as_str());
        println!("cargo::rerun-if-changed=tests/test_libs/src/{filename}.cpp");
    }
}

