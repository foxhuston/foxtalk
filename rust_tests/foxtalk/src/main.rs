mod auto_file_handler;

use std::thread::sleep;
use std::time::Duration;
use reactor::reactor::*;
use reactor::tuple::{Tuple, test_helpers};
use crate::auto_file_handler::AutoFileHandler;

use anyhow::Result;

fn main() -> Result<()> {
    let mut reactor = Reactor::new();

    println!("Adding boot claims...");
    reactor.claim(test_helpers::mk_tuple("/dev/video0", "is a", "camera"));

    let camera_format_handler = AutoFileHandler::new("./c/CameraFormatHandler.cpp", "./c/libcamera_format_handler.so")?;
    let camera_pixel_handler = AutoFileHandler::new("./c/CameraPixelHandler.cpp", "./c/libcamera_pixel_handler.so")?;

    println!("Adding boot handlers...");
    reactor.add_handler(Box::new(camera_format_handler));
    reactor.add_handler(Box::new(camera_pixel_handler));

    println!("Foxtalk Start!");

    while true {
        reactor.tick();
        // std::thread::sleep(Duration::from_millis(1));
        std::thread::sleep(Duration::from_secs(1));
    }

    Ok(())
}
