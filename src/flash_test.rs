use embedded_hal::timer::CountDown;
use fugit::ExtU32;
use core::fmt::Write;
use heapless::String;
use crate::thumby::Thumby;
use crate::thumby_flash::{DATA_START, read_flash_4096, write_flash_4096};
use crate::thumby_serial;

pub fn flash_test() -> ! {
    let thumby = Thumby::new();

    let mut serial_count_down = (&thumby.timer).count_down();
    serial_count_down.start(2_000.millis());

    let (mut usb_dev, mut serial) = thumby_serial::open_serial_port(&thumby.usb_bus_allocator);
    let mut said_hello = false;

    loop {
        if serial_count_down.wait().is_ok() {
            if !said_hello {
                said_hello = true;
                let _ = thumby_serial::write_serial_array(&mut serial, &mut b"Hello, World!\n");

                thumby_serial::write_serial_array(&mut serial, &mut "Program start\n");

                let addr = DATA_START;

                let mut text: String<64> = String::new();
                writeln!(&mut text, "Addr of flash block is {:x}\n", addr).unwrap();
                thumby_serial::write_serial(&mut serial, &mut text);

                let read_data: [u8; 4096] = read_flash_4096(addr);

                let mut text: String<64> = String::new();
                writeln!(&mut text, "Contents start with {} {} {} {}\n", read_data[0], read_data[1], read_data[2], read_data[3]).unwrap();
                thumby_serial::write_serial_array(&mut serial, &mut text);

                let mut data: [u8; 4096] = read_flash_4096(addr);
                data[0] = data[0].wrapping_add(1);
                core::sync::atomic::compiler_fence(core::sync::atomic::Ordering::SeqCst);
                write_flash_4096(addr, &data);
                core::sync::atomic::compiler_fence(core::sync::atomic::Ordering::SeqCst);
                thumby_serial::write_serial_array(&mut serial, &mut "Write done\n");

                let read_data: [u8; 4096] = read_flash_4096(addr);
                thumby_serial::write_serial_array(&mut serial, &mut "Read done 2\n");

                let mut text: String<64> = String::new();
                writeln!(&mut text, "Contents start with {} {} {} {}\n", read_data[0], read_data[1], read_data[2], read_data[3]).unwrap();
                thumby_serial::write_serial_array(&mut serial, &mut text);

                if read_data[0] != 0x56 {
                    panic!("unexpected");
                }
            } else {
                thumby_serial::write_serial_array(&mut serial, &mut "asdasd");
            }
        }
        thumby_serial::poll_to_dev_null(&mut usb_dev, &mut serial);
    }
}