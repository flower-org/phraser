#![no_std]
#![no_main]

mod thumby;
mod thumby_audio;
mod thumby_serial;
mod thumby_input;
mod thumby_keyboard;
mod keyboard_test;
mod keyboard_and_serial_test;

use bsp::entry;
use panic_halt as _;

use rp_pico as bsp;
use crate::keyboard_and_serial_test::keyboard_and_serial_test_main;

#[entry]
fn main() -> ! {
    keyboard_and_serial_test_main();
}
