#![no_std]
#![no_main]

mod thumby;
mod thumby_audio;
mod thumby_serial;
mod thumby_input;
mod thumby_keyboard;
mod keyboard_test;
mod keyboard_and_serial_test;
mod input_test;

use bsp::entry;
use heapless::String;
use panic_halt as _;

use rp_pico as bsp;
use crate::thumby::Thumby;

#[entry]
fn main() -> ! {
    let mut thumby = Thumby::new();
    input_test::input_test(&mut thumby);
}
