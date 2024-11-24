use std::ptr::null_mut;
use crash_handler::{jmp, CrashContext, CrashEvent, CrashEventResult};
use crash_handler::CrashEventResult::Jump;
use crate::reactor::ReactorProgramId;

#[derive(Clone)]
pub struct Safety {
    pub current: *mut jmp::JmpBuf,
    pub current_program_id: i32
}

impl Safety {
    pub fn new() -> Self {
        Self {
            current: std::mem::MaybeUninit::uninit().as_mut_ptr(),
            current_program_id: -1
        }
    }
    pub fn reset_jump_point(&mut self) {
        self.current = std::mem::MaybeUninit::uninit().as_mut_ptr();
    }

    pub fn set_program(&mut self, program_id: &ReactorProgramId) {
        self.current_program_id = program_id.0 as i32;
    }
}

unsafe impl Send for Safety {}
unsafe impl Sync for Safety {}

unsafe impl CrashEvent for Safety {
    fn on_crash(&self, _context: &CrashContext) -> CrashEventResult {
        Jump {
            jmp_buf: self.current,
            value: self.current_program_id
        }
    }
}