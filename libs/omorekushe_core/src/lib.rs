//! Omor Ekushe Rust & SIMD Acceleration Core
//! High-performance phonetic parsing, case normalization, predictive autocomplete, and macro expansion.

pub mod ffi;
pub mod macro_expander;
pub mod phonetic;
pub mod predict;
pub mod simd_diff;
pub mod spelling;

pub use ffi::*;
pub use macro_expander::*;
pub use phonetic::*;
pub use predict::*;
pub use simd_diff::*;
pub use spelling::*;
