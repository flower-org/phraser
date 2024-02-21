use cortex_m::asm;
use embedded_graphics::{
    pixelcolor::BinaryColor,
    prelude::*,
    primitives::{Circle, PrimitiveStyleBuilder},
};
use thumby::Thumby;

pub fn graphic_test() -> ! {
    let mut thumby = Thumby::new();

    let style = PrimitiveStyleBuilder::new()
        .stroke_width(1)
        .stroke_color(BinaryColor::On)
        .build();

    for i in (4..88).step_by(4) {
        Circle::new(Point::new(36 - i / 2, 20 - i / 2), i as u32)
            .into_styled(style)
            .draw(&mut thumby.display)
            .unwrap();
    }

    thumby.display.flush().unwrap();

    loop {
        asm::wfe();
    }
}