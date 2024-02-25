use core::ptr;
use rp2040_flash::flash;

pub const FLASH_START: u32 = 0x10000000;
pub const DATA_START: u32 = FLASH_START + 0x80000;

pub fn read_flash_4096(addr: u32) -> [u8; 4096] {
    let mut buffer: [u8; 4096] = [0; 4096]; // Initialize a buffer to store the read data
    unsafe {
        ptr::copy_nonoverlapping(addr as *const u8, buffer.as_mut_ptr(), 4096); // Read 4096 bytes from the memory address
    }
    buffer
}

pub fn write_flash_4096(addr: u32, data: &[u8; 4096]) {
    let addr_write = addr - 0x10000000;
    assert_eq!(addr_write & 0xfff, 0);//mut be a multiple of 4096

    cortex_m::interrupt::free(|_cs| {
        unsafe {
            flash::flash_range_erase_and_program(addr_write, data, true);
        }
    });
}
