use heapless::String;
use usb_device::bus::{UsbBus, UsbBusAllocator};
use usb_device::device::{UsbDevice, UsbDeviceBuilder, UsbVidPid};
use usbd_serial::SerialPort;

pub fn poll_to_dev_null<B: UsbBus>(usb_dev: &mut UsbDevice<B>, serial: &mut SerialPort<B>) -> () {
    // Check for new data
    if usb_dev.poll(&mut [serial]) {
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

pub fn open_serial_port<B: UsbBus>(usb_bus_allocator: &UsbBusAllocator<B>) -> (UsbDevice<B>, SerialPort<B>) {
    // Set up the USB Communications Class Device driver
    let serial = SerialPort::new(usb_bus_allocator);

    // Create a USB device with a fake VID and PID
    let usb_dev = UsbDeviceBuilder::new(usb_bus_allocator, UsbVidPid(0x16c0, 0x27dd))
        .manufacturer("flower-org")
        .product("Phraser Serial port")
        .serial_number("Phraser-9000")
        .device_class(2) // from: https://www.usb.org/defined-class-codes
        .build();

    (usb_dev, serial)
}

pub fn write_serial_array<B: UsbBus, T: AsRef<[u8]>>(serial: &mut SerialPort<B>, text: T) -> () {
    // This only works reliably because the number of bytes written to
    // the serial port is smaller than the buffers available to the USB
    // peripheral. In general, the return value should be handled, so that
    // bytes not transferred yet don't get lost.
    let _ = serial.write(text.as_ref());
}

pub fn write_serial<B: UsbBus>(serial: &mut SerialPort<B>, text: &mut String<64>) -> () {
    // This only works reliably because the number of bytes written to
    // the serial port is smaller than the buffers available to the USB
    // peripheral. In general, the return value should be handled, so that
    // bytes not transferred yet don't get lost.
    let _ = serial.write(text.as_bytes());
}
