//! Omor Ekushe Rust & SIMD Acceleration Core
//! High-performance phonetic parsing, case normalization, and vectorized diffing.

pub mod ffi;
pub mod phonetic;
pub mod simd_diff;
pub mod spelling;

pub use ffi::*;
pub use phonetic::*;
pub use simd_diff::*;
pub use spelling::*;

