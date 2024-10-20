// use crate::types::{ReactorHandler};
// pub struct Reactor<O, Q>
//     where Q: FnOnce(O) -> bool {
//     pub handlers: Vec<ReactorHandler<O, Q, _, _>>
// }
//
// impl<O, Q> Reactor<O, Q> {
//     pub fn new() -> Self {
//         Reactor {
//             handlers: Vec::new()
//         }
//     }
//
//     // pub fn add_handler(&mut self, handler: ReactorHandler<O, Q, H>) {
//     //     self.handlers.push(handler);
//     // }
//
//     pub fn add_input(&mut self, input: O) {
//         println!("adding o: {:?}", input);
//         println!("Todo: Actually add")
//     }
//
//     pub fn remove_input(&mut self, input: O) {
//         println!("adding o: {:?}", input);
//         println!("Todo: Actually add")
//     }
//
//     pub fn tick(&mut self) {
//         println!("tick tick tick");
//         println!("Todo: tick")
//     }
//
//
// }
//
// #[cfg(test)]
// mod tests {
//     use std::collections::HashSet;
//     use super::*;
//
//     #[test]
//     pub fn paper_example() {
//
//
//         struct PaperQueryHandler {
//             pub handle_calls:  Vec<u64>
//         }
//         impl PaperQueryHandler {
//             pub fn new() -> Self {
//                 PaperQueryHandler {
//                     handle_calls: Vec::new()
//                 }
//             }
//         }
//
//         struct PaperQuery;
//
//         impl Queryable<u64> for PaperQuery {
//             fn matches(self, other: u64) -> bool {
//                 other == 1 || other == 2
//             }
//         }
//
//         impl Handler<u64> for PaperQueryHandler {
//             fn handle(mut self, o: u64) -> HashSet<u64> {
//                 self.handle_calls.push(o);
//                 match o {
//                     1 => {
//                         let mut results = HashSet::new();
//                         results.insert(2);
//                         results.insert(3);
//                         results
//                     }
//                     2 => {
//                         let mut results = HashSet::new();
//                         results.insert(5);
//                         results
//                     }
//                     _ => HashSet::new()
//                 }
//             }
//
//             fn results(self) -> HashSet<(u64, u64)> {
//                 self.results()
//             }
//         }
//
//         let handler = ReactorHandler::new(PaperQuery{}, PaperQueryHandler::new());
//
//         let mut reactor = Reactor::new();
//         reactor.add_handler(handler);
//
//         // So if we call: 𝗂𝗇𝗌𝖾𝗋𝗍(𝔇0, 𝑜1), then...
//         reactor.add_input(1);
//
//         let reading_handler = &reactor.handlers[0];
//
//
//
//
//     }
// }