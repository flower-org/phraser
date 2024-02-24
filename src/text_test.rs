use cortex_m::asm;
use embedded_graphics::{
    pixelcolor::BinaryColor,
    prelude::*,
};
use crate::thumby::Thumby;
use embedded_graphics::{
    mono_font::{ascii::FONT_6X10, MonoTextStyle},
    text::Text,
};

pub fn text_test_main() -> ! {
    let mut thumby = Thumby::new();
    text_test(&mut thumby);
    loop {
        asm::wfe();
    }
}

pub fn text_test(thumby: &mut Thumby) -> () {
    let style = MonoTextStyle::new(&FONT_6X10, BinaryColor::On);
    Text::new("Hello Thumby", Point::new(0,20), style)
        .draw(&mut thumby.display)
        .unwrap();

    thumby.display.flush().unwrap();
}