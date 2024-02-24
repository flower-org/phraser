//! # Pico USB Serial Example
//!
//! Creates a USB Serial device on a Pico board, with the USB driver running in
//! the main thread.
//!
//! This will create a USB Serial device echoing anything it receives. Incoming
//! ASCII characters are converted to upercase, so you can tell it is working
//! and not just local-echo!
//!
//! See the `Cargo.toml` file for Copyright and license details.

#![no_std]
#![no_main]

mod thumby;
mod thumby_audio;
mod thumby_input;
mod thumby_serial;
mod text_test;
mod music_test;
mod serial_test;

// Ensure we halt the program on panic (if we don't mention this crate it won't
// be linked)
use panic_halt as _;

// The macro for our start-up function
use rp_pico::entry;
use crate::thumby::Thumby;

/// Entry point to our bare-metal application.
#[entry]
fn main() -> ! {
    let mut thumby = Thumby::new();

    text_test::text_test(&mut thumby);
    serial_test::serial_test(&mut thumby)
}
