use heapless::String;
use crate::thumby::Thumby;
use crate::thumby_serial;

pub fn input_test_main() -> ! {
    let mut thumby = Thumby::new();
    input_test(&mut thumby);
}

pub fn input_test(thumby: &mut Thumby) -> ! {
    //serial
    let (mut usb_dev, mut serial) = thumby_serial::open_serial_port(&thumby.usb_bus_allocator);


    let input = &mut thumby.input;

    loop {
        input.update();
        let mut result = String::new();

        if (input.left.pressed()) {
            result.push_str("left");
            result.push(',');
        }
        if (input.right.pressed()) {
            result.push_str("right");
            result.push(',');
        }
        if (input.up.pressed()) {
            result.push_str("up");
            result.push(',');
        }
        if (input.down.pressed()) {
            result.push_str("down");
            result.push(',');
        }
        if (input.a.pressed()) {
            result.push_str("a");
            result.push(',');
        }
        if (input.b.pressed()) {
            result.push_str("b");
            result.push(',');
        }

        let _ = thumby_serial::write_serial(&mut serial, &mut result);

        //serial poll
        thumby_serial::poll_to_dev_null(&mut usb_dev, &mut serial);
    }
}
