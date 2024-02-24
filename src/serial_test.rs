use crate::thumby::Thumby;
use crate::thumby_serial;

// Used to demonstrate writing formatted strings
use core::fmt::Write;
use heapless::String;

pub fn serial_test_main() -> ! {
    let mut thumby = Thumby::new();
    serial_test(&mut thumby);
}

pub fn serial_test(thumby: &mut Thumby) -> ! {
    let (mut usb_dev, mut serial) = thumby_serial::open_serial_port(&thumby.usb_bus_allocator);

    let mut said_hello = false;
    let mut previous = 0;
    loop {
        // A welcome message at the beginning
        if thumby.timer.get_counter().ticks() - previous > 2_000_000 {
            previous = previous + 2_000_000;
            if !said_hello {
                said_hello = true;
                let _ = thumby_serial::write_serial_array(&mut serial, &mut b"Hello, World!\n");
            }

            let time = thumby.timer.get_counter().ticks();
            let mut text: String<64> = String::new();
            writeln!(&mut text, "Current timer ticks: {}", time).unwrap();
            let _ = thumby_serial::write_serial(&mut serial, &mut text);
        }

        thumby_serial::poll_to_dev_null(&mut usb_dev, &mut serial);
    }
}