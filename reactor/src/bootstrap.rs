// use std::str::FromStr;
// use indradb::{Identifier, MemoryDatastore};
//
// use indradb::Edge;
//
// #[cfg(test)]
// mod tests {
//     use indradb::{Query, SpecificEdgeQuery};
//     use super::*;
//
//     #[test]
//     fn test_startup() {
//         let db = MemoryDatastore::new_db();
//         let system = Identifier::from_str("system").unwrap();
//         let wants_to_load = Identifier::from_str("wants_to_load").unwrap();
//         let library_loader = Identifier::from_str("library_loader").unwrap();
//
//         let system_vertex = db.create_vertex_from_type(system).unwrap();
//         let lib_loader = db.create_vertex_from_type(library_loader).unwrap();
//         let wants_to_load_edge = Edge::new(system_vertex, wants_to_load, lib_loader);
//
//         let e = db.create_edge(&wants_to_load_edge).unwrap();
//
//         let query = Query::SpecificEdge(SpecificEdgeQuery::single(wants_to_load_edge));
//         let res = db.get(query).unwrap();
//
//         println!("{:?}", res);
//     }
// }