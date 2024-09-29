mod auto_file_handler;

use std::thread::sleep;
use std::time::Duration;
use reactor::reactor::*;
use reactor::tuple::Tuple;
use crate::auto_file_handler::AutoFileHandler;

use anyhow::Result;

fn main() -> Result<()> {
    let mut reactor = Reactor::new();

    println!("Adding boot claims...");
    reactor.claim(Tuple::new_strs("/dev/video0", "is a", "camera"));

    let afh = AutoFileHandler::new("./c/CameraHandler.cpp", "./c/libcamera_handler.so")?;
    println!("Adding boot handlers...");
    reactor.add_handler(Box::new(afh));

    println!("Foxtalk Start!");

    while true {
        reactor.tick();
        // std::thread::sleep(Duration::from_millis(1));
        std::thread::sleep(Duration::from_secs(1));
    }

    Ok(())
}
