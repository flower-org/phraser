#![no_std]
#![no_main]

mod thumby;
mod thumby_audio;
mod thumby_input;
mod thumby_keyboard;
mod keyboard_test;

use bsp::entry;
use cortex_m::prelude::*;
use embedded_hal::digital::v2::*;
use fugit::ExtU32;
#[allow(clippy::wildcard_imports)]
use usb_device::class_prelude::*;
use usbd_human_interface_device::page::Keyboard;
use panic_halt as _;

use rp_pico as bsp;
use crate::keyboard_test::{keyboard_test, keyboard_test_main};
use crate::thumby::Thumby;
use crate::thumby_keyboard::{press_key, release_keys, start_usb_keyboard, tick_and_poll_to_dev_null};

#[entry]
fn main() -> ! {
//    let mut thumby = Thumby::new();
    keyboard_test_main();
}
