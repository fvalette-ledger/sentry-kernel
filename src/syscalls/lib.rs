#![no_std]

pub mod arch;
pub mod gate;

#[cfg(debug_assertions)]
#[macro_use]
pub mod debug;

pub mod sysimpl;
use sysimpl::*;

#[cfg(feature = "mock")]
pub mod mocks;

#[cfg(not(feature = "mock"))]
#[panic_handler]
fn panic(_: &core::panic::PanicInfo) -> ! {
    loop {}
}