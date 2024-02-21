use cortex_m::asm;
use thumby::music::{A, C, D, E, F, G, REST};
use thumby::Thumby;

const TWINKLE_TWINKLE: [f32; 48] = [
    C, C, G, G, A, A, G, REST,
    F, F, E, E, D, D, C, REST,
    G, G, F, F, E, E, D, REST,
    G, G, F, F, E, E, D, REST,
    C, C, G, G, A, A, G, REST,
    F, F, E, E, D, D, C, REST,
];

pub fn music_test() -> ! {
    let mut thumby = Thumby::new();

    for note in TWINKLE_TWINKLE {
        thumby.audio.play(note);
        thumby.wait_ms(500);
        thumby.audio.stop();
        thumby.wait_ms(100);
    }

    loop {
        asm::wfe();
    }
}
