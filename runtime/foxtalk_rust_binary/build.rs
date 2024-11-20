fn main() {
    // println!("cargo:rustc-link-arg=-Wl");
    println!("cargo:rustc-link-arg=-Bstatic");
    println!("cargo:rustc-link-arg=-lopencv_core");
}