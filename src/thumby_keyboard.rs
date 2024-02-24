use frunk::{HCons, HNil};
use usb_device::bus::{UsbBus, UsbBusAllocator};
use usb_device::device::{UsbDevice, UsbDeviceBuilder, UsbVidPid};
use usb_device::UsbError;
use usbd_human_interface_device::device::DeviceHList;
use usbd_human_interface_device::device::keyboard::BootKeyboard;
use usbd_human_interface_device::page::Keyboard;
use usbd_human_interface_device::prelude::*;

pub fn tick_and_poll_to_dev_null<'a, B: UsbBus>(usb_dev: &mut UsbDevice<'a, B>, keyboard: &mut UsbHidClass<'a, B, HCons<BootKeyboard<'a, B>, HNil>>) -> () {
    // Check for new data
    match keyboard.tick() {
        Err(UsbHidError::WouldBlock) => {}
        Ok(_) => {}
        Err(e) => {
            core::panic!("Failed to process keyboard tick: {:?}", e)
        }
    };

    if usb_dev.poll(&mut [keyboard]) {
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
    }
}

pub fn start_usb_keyboard<B: UsbBus>(usb_bus_allocator: &UsbBusAllocator<B>) -> (UsbDevice<B>, UsbHidClass<B, HCons<BootKeyboard<B>, HNil>>) {
    // Set up the USB Communications Class Device driver
    let mut keyboard = UsbHidClassBuilder::new()
        .add_device(usbd_human_interface_device::device::keyboard::BootKeyboardConfig::default())
        .build(usb_bus_allocator);

    // Create a USB device with a fake VID and PID
    let mut usb_dev = UsbDeviceBuilder::new(usb_bus_allocator, UsbVidPid(0x1209, 0x0001))
        .manufacturer("flower-org")
        .product("Boot Keyboard")
        .serial_number("Phraser")
        .build();

    (usb_dev, keyboard)
}

pub fn press_key<'a, B: UsbBus>(key: Keyboard, keyboard: &mut UsbHidClass<'a, B, HCons<BootKeyboard<'a, B>, HNil>>) -> () {
    let keys = get_keys(key, &true);
    write_keys(keys, keyboard);
}

pub fn release_keys<'a, B: UsbBus>(keyboard: &mut UsbHidClass<'a, B, HCons<BootKeyboard<'a, B>, HNil>>) -> () {
    let keys = get_keys(Keyboard::A, &false);
    write_keys(keys, keyboard);
}

pub fn write_keys<'a, B: UsbBus, K: IntoIterator<Item = Keyboard>>(keys: K, keyboard: &mut UsbHidClass<'a, B, HCons<BootKeyboard<'a, B>, HNil>>) -> () {
    match keyboard.device().write_report(keys) {
        Err(UsbHidError::WouldBlock) => {}
        Err(UsbHidError::Duplicate) => {}
        Ok(_) => {}
        Err(e) => {
            core::panic!("Failed to write keyboard report: {:?}", e)
        }
    };
}

fn get_keys(key: Keyboard, is_pressed: &bool) -> [Keyboard; 6] {
    [
        if *is_pressed { key } else { Keyboard::NoEventIndicated },
        Keyboard::NoEventIndicated,
        Keyboard::NoEventIndicated,
        Keyboard::NoEventIndicated,
        Keyboard::NoEventIndicated,
        Keyboard::NoEventIndicated,
    ]
}

/*pub fn type_keyboard_array<B: UsbBus, T: AsRef<[u8]>>(serial: &mut SerialPort<B>, text: T) -> () {
    // This only works reliably because the number of bytes written to
    // the serial port is smaller than the buffers available to the USB
    // peripheral. In general, the return value should be handled, so that
    // bytes not transferred yet don't get lost.
    serial.write(text.as_ref());
}

pub fn type_keyboard_serial<B: UsbBus>(serial: &mut SerialPort<B>, text: &mut String<64>) -> () {
    // This only works reliably because the number of bytes written to
    // the serial port is smaller than the buffers available to the USB
    // peripheral. In general, the return value should be handled, so that
    // bytes not transferred yet don't get lost.
    serial.write(text.as_bytes());
}*/
