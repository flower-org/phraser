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
pub fn text_test() -> ! {
    let mut thumby = Thumby::new();

    let style = MonoTextStyle::new(&FONT_6X10, BinaryColor::On);
    Text::new("Hello Thumby", Point::new(0,20), style)
        .draw(&mut thumby.display)
        .unwrap();

    thumby.display.flush().unwrap();

    loop {
        asm::wfe();
    }
}