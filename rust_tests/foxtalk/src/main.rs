mod auto_file_handler;

use std::thread::sleep;
use std::time::Duration;
use reactor::reactor::*;
use reactor::tuple::Tuple;
use crate::auto_file_handler::AutoFileHandler;

use anyhow::Result;

#[tokio::main]
async fn main() -> Result<()> {
    let mut reactor = Reactor::new();

    println!("Adding boot claims...");
    reactor.claim(Tuple::new_strs("/dev/video0", "is a", "camera"));

    let afh = AutoFileHandler::new("./c/CameraHandler.cpp", "./c/libcamera_handler.so").await;

    println!("Adding boot handlers...");

    println!("Foxtalk Start!");

    while true {
        reactor.tick();
        tokio::time::sleep(Duration::from_millis(1));
    }

    Ok(())
}
