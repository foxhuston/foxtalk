use walkdir::WalkDir;

pub fn regenerate_compiler_commands() {
    let root_build_folder = std::env::var("SO_PATH").unwrap();
    let dirs = WalkDir::new(root_build_folder.clone());commands
    let mut commands = Vec::new();

    for entry in dirs.into_iter().filter_map(|e| e.ok()) {
        if let Some(file_name) = entry.file_name().to_str() {
            if file_name.ends_with(".json") && file_name != "compile_commands.json" {
                let file_contents = std::fs::read_to_string(entry.path()).unwrap();
                commands.push(file_contents)
            }
        }
    }
    let commands = "[".to_string() + commands.join("\n").as_str();
    let mut final_commands = commands.rsplit_once(",").unwrap().0.to_string();
    final_commands.push(']');
    std::fs::write(root_build_folder + "/" +"compile_commands.json", final_commands).unwrap
    ()
}