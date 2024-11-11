use std::ptr::slice_from_raw_parts_mut;
use rust_tuple_reactor_serde::tuple::Tuple;
use rust_tuple_reactor_serde::tuple_noun::TupleNoun;
use rust_tuple_reactor_serde::*;
use std::sync::LazyLock;

use eframe::{egui, EventLoopBuilderHook};
use winit::platform::wayland::EventLoopBuilderExtWayland;

static BUFFER_SIZE: usize = 10 * 1024 * 1024;

pub static GUI_THREAD_HANDLE: LazyLock<std::thread::JoinHandle<()>> =
    LazyLock::new(|| std::thread::spawn(|| {

        let event_loop_builder: Option<EventLoopBuilderHook> = Some(Box::new(|event_loop_builder| {
            event_loop_builder.with_any_thread(true);
        }));
        let native_options = eframe::NativeOptions {
            event_loop_builder,
            ..Default::default()
        };
        eframe::run_simple_native("App", native_options, update).expect("failed to run app");
    }));

static mut LATEST_SEEN_TUPLE: [Option<Tuple>; 1] = [None];

fn update(ctx: &eframe::egui::Context, _: &mut eframe::Frame) {
    egui::CentralPanel::default().show(ctx, |ui| {
        ui.heading("hello");

        unsafe {
            if let Some(Tuple(nouns)) = &LATEST_SEEN_TUPLE[0] {
                ui.label(format!("{:?}", &nouns[2]));
            }
        }
    });
}

#[no_mangle]
pub extern "C" fn init(buffer: *mut u8) {
    let query_tuple = vec![Tuple(vec![
        TupleNoun::Symbol("foxtalk reactor".to_string()),
        TupleNoun::Symbol("sees tuples".to_string()),
        TupleNoun::Query
    ])];
    unsafe { match slice_from_raw_parts_mut(buffer, BUFFER_SIZE).as_mut() {
        None => { eprintln!("Slice from raw parts failed") }
        Some(buf) => {query_tuple.iter().write_to_buffer(buf, 0); }
    }};
    println!("Is about the start the gui thread!");
    let _ = GUI_THREAD_HANDLE.is_finished();
    println!("Should have stared the gui thread!");

}
#[no_mangle]
pub extern "C" fn free_tuple(_buffer: *mut u8) {

}
#[no_mangle]
pub extern "C" fn handle(buffer: *mut u8) {
    unsafe { match slice_from_raw_parts_mut(buffer, BUFFER_SIZE).as_mut() {
        None => { eprintln!("Slice from raw parts failed") }
        Some(buf) => {
            let (tuples, _) = unsafe { Vec::<Tuple>::read_from_buffer(buf, 0) };
            if !tuples.is_empty() {
                unsafe { LATEST_SEEN_TUPLE[0] = Some(tuples[0].clone()); }
            }
        }
    }};

}
#[no_mangle]
pub extern "C" fn register_initial_tuples(_buffer: *mut [u8]) {
}
#[no_mangle]
pub extern "C" fn teardown() {
}
#[no_mangle]
pub extern "C" fn poll() -> bool {
    false
}
