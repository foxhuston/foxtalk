pub mod non_aggregating_handler;

#[cfg(test)]
pub mod test {

    use std::path::PathBuf;

    pub fn linked_lib_path(filename: &str) -> PathBuf {
        let mut path = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
        path.push("tests/test_libs/out");
        path.push(filename);
        let new_path = path.clone();
        let path_str = new_path.to_str().unwrap();
        let owned_path = path_str;
        PathBuf::from(owned_path)
    }

}