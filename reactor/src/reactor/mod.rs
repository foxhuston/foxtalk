pub mod utils;
pub mod query_engine;
pub mod reactor_program;

use crate::reactor::query_engine::QueryEngine;
use crate::reactor::reactor_program::{Program, ProgramInfo};
use rustc_hash::{FxHashMap, FxHashSet};
use std::fmt::Debug;
use std::hash::Hash;

#[derive(PartialEq, Eq, Hash, Clone, Debug)]
#[repr(transparent)]
pub struct ReactorProgramId(u64);


pub trait GeneratesProgram<Q>
where Self: Eq + Hash + Sized {
    fn mk_handler_with_bootstrap_input(&self) -> Option<(Box<dyn Program<Self, Q>>, FxHashSet<Self>)> { None }
}

impl<Q> GeneratesProgram<Q> for u64 { }

pub trait ReactorData<Q> where Self: Eq + Hash + Clone + Debug + Send + GeneratesProgram<Q> {}

pub struct Reactor<Q, QE, O>
    where
        O: ReactorData<Q>,
        QE : QueryEngine<O, ReactorProgramId, Q>
{
    pub program_map: FxHashMap<ReactorProgramId, ProgramInfo<O>>,
    reactor_program_map: FxHashMap<ReactorProgramId, Box<dyn Program<O, Q>>>,
    pub ref_counts: FxHashMap<O, u64>,

    query_engine: QE,
    
    generated_programs: FxHashMap<O, ReactorProgramId>,
    current_program_id: u64,
}

impl<Q, O: ReactorData<Q>, QE: QueryEngine<O, ReactorProgramId, Q>> Reactor<Q, QE, O> {
    pub fn new(query_engine: QE) -> Self {
        Reactor {
            program_map: FxHashMap::default(),
            reactor_program_map: FxHashMap::default(),
            ref_counts: FxHashMap::default(),
            query_engine,
            generated_programs: FxHashMap::default(),
            current_program_id: 0,
        }
    }

    #[allow(non_snake_case)]
    pub fn add_program_with_bootstrap_output(&mut self, mut program: Box<dyn Program<O, Q>>, O: FxHashSet<O>) -> ReactorProgramId {

        // let mut handler = ReactorHandler::new_with_bootstrap_output(program, O);

        let mut program_info = ProgramInfo {
            I: FxHashSet::default(),
            O,
            dirty: true,
        };

        for output in &program_info.O {
            self.insert(output.clone());
        }

        let id = ReactorProgramId(self.current_program_id);
        self.query_engine.insert_program_for_query(program.query(), id.clone());

        let all_outputs: FxHashSet<&O> = self.ref_counts.keys().collect();
        for output in all_outputs {
            if self.query_engine.query(output).contains(&id) {
                program_info.I.insert(output.clone());
            }
        }
        program_info.dirty = true;

        self.current_program_id += 1;
        self.program_map.insert(id.clone(), program_info);
        self.reactor_program_map.insert(id.clone(), program);
        
        id
    }

    pub fn add_program(&mut self, program: Box<dyn Program<O, Q>>) -> ReactorProgramId {
        self.add_program_with_bootstrap_output(program, FxHashSet::default())
    }
    
    pub fn remove_program(&mut self, handler_id: ReactorProgramId) {
        // println!("Removing handler {:?}", handler_id);

        if let Some(handler) = self.program_map.get(&handler_id) {
            let output = handler.O.clone();
            for o in output {
                // println!("Because of removing handler {:?}, removing o: {:?}", handler_id, o);
                self.remove_with_hid(o, Some(handler_id.clone()));
            }
        };
        // println!("Removing handler {:?} from the reactor hashset", handler_id);
        self.program_map.remove(&handler_id);
        self.reactor_program_map.remove(&handler_id);
    }

    pub fn insert(&mut self, input: O) {
        // println!("adding o: {:?}", input);
        if self.ref_counts.contains_key(&input) {
            let count = self.ref_counts.get_mut(&input).unwrap();
            *count += 1;
        } else {
            if let Some((h, o)) = input.mk_handler_with_bootstrap_input() {
                let hid = self.add_program_with_bootstrap_output(h, o);
                self.generated_programs.insert(input.clone(), hid);
            }

           for program_id in self.query_engine.query(&input) {
                let program = self.program_map.get_mut(&program_id).unwrap();
                if !program.I.contains(&input) {
                    program.I.insert(input.clone());
                    program.dirty = true;
                }
           }

            self.ref_counts.insert(input, 1);
        }
    }


    pub fn remove(&mut self, input: O) {
        self.remove_with_hid(input, None);
    }
    fn remove_with_hid(&mut self, input: O, maybe_hid: Option<ReactorProgramId>) {
        // println!("removing o: {:?}", input);
        if self.ref_counts.contains_key(&input) {
            let count = self.ref_counts.get_mut(&input).unwrap();
            *count -= 1;
            if *count == 0 {
                if let Some(hid) = maybe_hid {
                    self.reactor_program_map.get_mut(&hid).unwrap().free_o(&input);
                }
                // println!("Am I in the generated handlers list? {:?}", input);
                if let Some(hid) = self.generated_programs.remove(&input) {
                    // println!("Yes!");
                    self.remove_program(hid);
                }
                
                self.ref_counts.remove(&input);
                
                for (_, handler) in self.program_map.iter_mut() {
                    if handler.I.contains(&input) {
                        handler.I.remove(&input);
                        handler.dirty = true;
                    }
                }
            }
        }
        // println!("Done removing o: {:?}", input);
    }

    pub fn tick(&mut self) {
        // println!("tick tick tick");
        let mut need_to_insert_total = Vec::new();
        let mut need_to_remove_total = Vec::new();
        for (hid, handler) in self.program_map.iter_mut() {
            if handler.dirty {
                // let qa = &mut handler.qa;
                let input = &handler.I;

                let program = self.reactor_program_map.get_mut(&hid).unwrap();
                let new_output = program.handle(input);

                let need_to_insert: FxHashSet<O> = new_output.difference(&handler.O).cloned().collect();
                let need_to_remove: FxHashSet<O> = handler.O.difference(&new_output).cloned().collect();

                for i in need_to_insert.into_iter() {
                    need_to_insert_total.push(i);
                }

                for i in need_to_remove.into_iter() {
                    need_to_remove_total.push((i, hid.clone()));
                }
                handler.O = new_output;
                handler.dirty = false;

            }

        }

        for i in need_to_insert_total.into_iter() {
            self.insert(i);
        }
            
        for (i, hid) in need_to_remove_total.into_iter() {
            self.remove_with_hid(i, Some(hid));
        }
    }


}

impl<Q, O: ReactorData<Q>, QE: QueryEngine<O, ReactorProgramId, Q>>  Drop for Reactor<Q, QE, O> {
    fn drop(&mut self) {
        // println!("Dropping the reactor now!");
        let hids: Vec<ReactorProgramId> = self.program_map.keys().cloned().collect();

        for hid in hids {
            // println!("Dropping handler {:?} because we're dropping reactor!", hid);
            self.remove_program(hid);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::reactor::query_engine::{SimpleQuery, SimpleQueryEngine};

    #[test]
    pub fn paper_agg_example() {
        struct PaperHandler {}

        #[derive(PartialEq, Eq, Clone, Debug, Hash)]
        struct PaperQuery();

        impl SimpleQuery<u64> for PaperQuery {
            fn query(&self, o: &u64) -> bool {
                *o > 5 && *o < 20
            }
        }
        impl ReactorData<PaperQuery> for u64{}

        impl Program<u64, PaperQuery> for PaperHandler {
            fn query(&mut self) -> PaperQuery {
                PaperQuery{}
            }

            fn handle(&mut self, o: &FxHashSet<u64>) -> FxHashSet<u64> {
                let mut sum = 0;
                for &i in o {
                    sum += i;
                }
                let mut out = FxHashSet::default();
                out.insert(sum);
                out
            }
        }

        let paper_query_engine: SimpleQueryEngine<ReactorProgramId, PaperQuery> = query_engine::SimpleQueryEngine::new();
        let mut reactor = Reactor::new(paper_query_engine);
        let handler = Box::new(PaperHandler{});
        let hid = reactor.add_program(handler);

        reactor.query_engine.insert_program_for_query(PaperQuery{}, hid.clone());

        reactor.tick();
        reactor.insert(3);
        reactor.insert(7);
        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&3).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);
        assert_eq!(reactor.ref_counts.get(&7).is_some(), true);
        assert_eq!(reactor.ref_counts.get(&7).unwrap(), &2);

        assert_eq!(reactor.program_map.get(&hid).unwrap().dirty, false);
        reactor.insert(10);
        assert_eq!(reactor.program_map.get(&hid).unwrap().dirty, true);
        assert_eq!(reactor.program_map.get(&hid).unwrap().I.contains(&10), true);
        reactor.tick();
        assert_eq!(reactor.ref_counts.get(&3).unwrap(), &1);
    }
}