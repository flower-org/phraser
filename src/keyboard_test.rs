use embedded_hal::prelude::_embedded_hal_timer_CountDown;
use fugit::ExtU32;
use usbd_human_interface_device::page::Keyboard;
use crate::thumby::Thumby;
use crate::thumby_keyboard::{press_key, release_keys, start_usb_keyboard, tick_and_poll_to_dev_null};

pub fn keyboard_test_main() -> ! {
    let mut thumby = Thumby::new();
    keyboard_test(&mut thumby);
}

pub fn keyboard_test(thumby: &mut Thumby) -> ! {
    // Set up the USB Communications Class Device driver
    let (mut usb_dev, mut keyboard) = start_usb_keyboard(&thumby.usb_bus_allocator);

    //Timers
    let mut input_count_down = (&thumby.timer).count_down();
    input_count_down.start(10.millis());

    let mut tick_count_down = (&thumby.timer).count_down();
    tick_count_down.start(1.millis());

    let mut pressed = false;
    loop {
        //Poll the keys every 10ms
        if input_count_down.wait().is_ok() {
            if pressed { press_key(Keyboard::A, &mut keyboard) } else { release_keys(&mut keyboard) }
            pressed = !pressed;
        }

        //Tick once per ms
        if tick_count_down.wait().is_ok() {
            tick_and_poll_to_dev_null(&mut usb_dev, &mut keyboard);
        }
    }
}