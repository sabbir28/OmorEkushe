//! Foreign Function Interface (FFI) bindings for C and C++ integration.

use crate::phonetic;
use crate::simd_diff;
use crate::spelling;
use core::slice;

#[no_mangle]
pub unsafe extern "C" fn omorekushe_fast_simd_prefix(
    s1: *const u16,
    len1: usize,
    s2: *const u16,
    len2: usize,
) -> usize {
    if s1.is_null() || s2.is_null() || len1 == 0 || len2 == 0 {
        return 0;
    }
    let slice1 = slice::from_raw_parts(s1, len1);
    let slice2 = slice::from_raw_parts(s2, len2);
    simd_diff::common_prefix_utf16(slice1, slice2)
}

#[no_mangle]
pub unsafe extern "C" fn omorekushe_fast_compute_diff(
    old_buf: *const u16,
    old_len: usize,
    new_buf: *const u16,
    new_len: usize,
    out_common: *mut usize,
    out_backspaces: *mut usize,
) -> usize {
    let common = if old_buf.is_null() || new_buf.is_null() || old_len == 0 || new_len == 0 {
        0
    } else {
        let slice_old = slice::from_raw_parts(old_buf, old_len);
        let slice_new = slice::from_raw_parts(new_buf, new_len);
        simd_diff::common_prefix_utf16(slice_old, slice_new)
    };

    if !out_common.is_null() {
        *out_common = common;
    }
    if !out_backspaces.is_null() {
        *out_backspaces = old_len - common;
    }

    // Returns the offset where the new characters to send begin
    common
}

#[no_mangle]
pub unsafe extern "C" fn omorekushe_fast_fix_string(
    input: *const u8,
    len: usize,
    out_buf: *mut u8,
    out_capacity: usize,
) -> usize {
    if input.is_null() || out_buf.is_null() || len == 0 || out_capacity == 0 {
        return 0;
    }
    let to_copy = len.min(out_capacity);
    let in_slice = slice::from_raw_parts(input, to_copy);
    let out_slice = slice::from_raw_parts_mut(out_buf, to_copy);

    for i in 0..to_copy {
        out_slice[i] = phonetic::CASE_MAP[in_slice[i] as usize];
    }
    to_copy
}

#[no_mangle]
pub extern "C" fn omorekushe_fast_is_vowel(c: u8) -> bool {
    phonetic::is_vowel(c)
}

#[no_mangle]
pub extern "C" fn omorekushe_fast_is_consonant(c: u8) -> bool {
    phonetic::is_consonant(c)
}

#[no_mangle]
pub unsafe extern "C" fn omorekushe_fast_levenshtein(
    a: *const u32,
    len_a: usize,
    b: *const u32,
    len_b: usize,
    max_dist: u32,
) -> u32 {
    if a.is_null() || b.is_null() {
        return max_dist + 1;
    }
    let slice_a = slice::from_raw_parts(a, len_a);
    let slice_b = slice::from_raw_parts(b, len_b);
    spelling::myers_levenshtein_64(slice_a, slice_b, max_dist)
}

#[no_mangle]
pub unsafe extern "C" fn omorekushe_fast_utf8_to_codepoints(
    utf8_ptr: *const u8,
    utf8_len: usize,
    out_cp: *mut u32,
    max_out: usize,
) -> usize {
    if utf8_ptr.is_null() || out_cp.is_null() || utf8_len == 0 || max_out == 0 {
        return 0;
    }
    let slice_bytes = slice::from_raw_parts(utf8_ptr, utf8_len);
    if let Ok(text) = core::str::from_utf8(slice_bytes) {
        let out_slice = slice::from_raw_parts_mut(out_cp, max_out);
        spelling::utf8_to_codepoints(text, out_slice)
    } else {
        0
    }
}

