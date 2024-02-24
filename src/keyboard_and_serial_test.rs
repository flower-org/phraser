use embedded_hal::prelude::_embedded_hal_timer_CountDown;
use fugit::ExtU32;
use core::fmt::Write;
use heapless::String;
use usb_device::device::{UsbDeviceBuilder, UsbVidPid};
use usb_device::UsbError;
use usbd_human_interface_device::page::Keyboard;
use usbd_human_interface_device::prelude::UsbHidClassBuilder;
use usbd_human_interface_device::UsbHidError;
use usbd_serial::SerialPort;
use crate::thumby::Thumby;
use crate::thumby_keyboard::{press_key, release_keys};
use crate::thumby_serial;

pub fn keyboard_and_serial_test_main() -> ! {
    let mut thumby = Thumby::new();

    let mut said_hello = false;


    let usb_bus_allocator = &thumby.usb_bus_allocator;

    // Set up the USB Communications Class Device driver
    let mut serial = SerialPort::new(usb_bus_allocator);

    let mut keyboard = UsbHidClassBuilder::new()
        .add_device(usbd_human_interface_device::device::keyboard::BootKeyboardConfig::default())
        .build(usb_bus_allocator);

    // Create a USB device with a fake VID and PID
    let mut usb_dev = UsbDeviceBuilder::new(usb_bus_allocator, UsbVidPid(0x16c0, 0x27dd))
        .manufacturer("flower-org")
        .product("Phraser")
        .serial_number("9000")
        .device_class(0x00) // from: https://www.usb.org/defined-class-codes
        .build();

    //Timers
    let mut input_count_down = (&thumby.timer).count_down();
    input_count_down.start(10.millis());

    let mut tick_count_down = (&thumby.timer).count_down();
    tick_count_down.start(1.millis());

    let mut serial_count_down = (&thumby.timer).count_down();
    serial_count_down.start(2_000.millis());

    let mut pressed = false;
    loop {
        //Poll the keys every 10ms
        if input_count_down.wait().is_ok() {
            if pressed { press_key(Keyboard::A, &mut keyboard) } else { release_keys(&mut keyboard) }
            pressed = !pressed;
        }

        if serial_count_down.wait().is_ok() {
            if !said_hello {
                said_hello = true;
                let _ = thumby_serial::write_serial_array(&mut serial, &mut b"Hello, World!\n");
            }

            let time = thumby.timer.get_counter().ticks();
            let mut text: String<64> = String::new();
            writeln!(&mut text, "Current timer ticks: {}", time).unwrap();
            let _ = thumby_serial::write_serial(&mut serial, &mut text);
        }

        //Tick once per ms
        if tick_count_down.wait().is_ok() {
            // Check for new data
            match keyboard.tick() {
                Err(UsbHidError::WouldBlock) => {}
                Ok(_) => {}
                Err(e) => {
                    core::panic!("Failed to process keyboard tick: {:?}", e)
                }
            };

            if usb_dev.poll(&mut [&mut keyboard, &mut serial]) {
                match keyboard.device().read_report() {
                    Err(UsbError::WouldBlock) => {
                        //do nothing
                    }
                    Err(e) => {
                        //do nothing
                    }
                    Ok(leds) => {
                        //do nothing
                    }
                }
                let mut buf = [0u8; 64];
                match serial.read(&mut buf) {
                    Err(_e) => {
                        // Do nothing
                    }
                    Ok(0) => {
                        // Do nothing
                    }
                    Ok(count) => {
                        // Convert to upper case
                        buf.iter_mut().take(count).for_each(|b| {
                            b.make_ascii_uppercase();
                        });
                        // Send back to the host
                        let mut wr_ptr = &buf[..count];
                        while !wr_ptr.is_empty() {
                            match serial.write(wr_ptr) {
                                Ok(len) => wr_ptr = &wr_ptr[len..],
                                // On error, just drop unwritten data.
                                // One possible error is Err(WouldBlock), meaning the USB
                                // write buffer is full.
                                Err(_) => break,
                            };
                        }
                    }
                }
            }
        }
    }
}
