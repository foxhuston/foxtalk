// use libloading::{Error, Library};
// 
// struct LoadedLibrary<'a> {
//     buffer: &'a mut [u8],
// }
// 
// impl<'a> LoadedLibrary<'a> {
//     pub fn load(lib_path: String) -> Result<Self, Error> {
//         let lib = unsafe { Library::new(lib_path)? };
// 
// 
// 
//         // let mut buffer = vec![0; 1024];
//         // let lib = std::fs::File::open(lib_path).unwrap();
//         // let lib_size = lib.metadata().unwrap().len();
//         // let mut lib = std::io::BufReader::new(lib);
//         // lib.read_exact(&mut buffer).unwrap();
//         // Self { buffer }
//         todo!()
//     }
// }