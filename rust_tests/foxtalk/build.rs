fn main() {
    // This is needed so that dynamically linked libraries can find
    // `pub extern "C" fn`s from the main rust code.
    println!("cargo::rustc-link-arg=-export-dynamic");
}