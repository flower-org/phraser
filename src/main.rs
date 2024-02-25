#![no_std]
#![no_main]

mod thumby;
mod thumby_serial;
mod thumby_audio;
mod thumby_input;
mod thumby_flash;
mod flash_test;

use rp_pico::entry;
use panic_halt as _;

#[entry]
fn main() -> ! {
    flash_test::flash_test();
}

