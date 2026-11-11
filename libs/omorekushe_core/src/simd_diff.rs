//! High-performance SIMD and x86_64 assembly string diff engine.
//! Computes UTF-16 common prefixes using 128-bit vectorization and inline assembly bit-scans.

#[cfg(target_arch = "x86_64")]
use core::arch::x86_64::*;

/// Native x86_64 assembly bit-scan forward wrapper.
/// Returns the bit position of the least significant 1-bit in `val`.
#[inline(always)]
#[cfg(target_arch = "x86_64")]
pub unsafe fn asm_bsf_u32(val: u32) -> u32 {
    let res: u32;
    core::arch::asm!(
        "bsf {res:e}, {val:e}",
        val = in(reg) val,
        res = out(reg) res,
        options(pure, nomem, nostack)
    );
    res
}

/// Computes the common prefix length between two UTF-16 string buffers.
/// Accelerates comparison by scanning 8 UTF-16 characters (128 bits) per iteration
/// using SSE2 vector instructions and assembly bit-scanning.
#[inline]
pub fn common_prefix_utf16(s1: &[u16], s2: &[u16]) -> usize {
    let min_len = s1.len().min(s2.len());
    let mut i = 0;

    #[cfg(target_arch = "x86_64")]
    {
        let p1 = s1.as_ptr();
        let p2 = s2.as_ptr();

        // Vectorized SIMD loop: 8 UTF-16 code units (16 bytes) at once
        while i + 8 <= min_len {
            unsafe {
                let v1 = _mm_loadu_si128(p1.add(i) as *const __m128i);
                let v2 = _mm_loadu_si128(p2.add(i) as *const __m128i);
                let cmp = _mm_cmpeq_epi16(v1, v2);
                let mask = _mm_movemask_epi8(cmp) as u32;

                if mask != 0xFFFF {
                    // Extract first mismatch using assembly BSF
                    let diff_mask = !mask & 0xFFFF;
                    let first_diff_byte = asm_bsf_u32(diff_mask) as usize;
                    return i + (first_diff_byte / 2);
                }
            }
            i += 8;
        }
    }

    // Scalar remainder
    while i < min_len && s1[i] == s2[i] {
        i += 1;
    }
    i
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_common_prefix() {
        let a: Vec<u16> = "Hello, World!".encode_utf16().collect();
        let b: Vec<u16> = "Hello, Omor Ekushe!".encode_utf16().collect();
        assert_eq!(common_prefix_utf16(&a, &b), 7); // "Hello, "

        let empty: Vec<u16> = vec![];
        assert_eq!(common_prefix_utf16(&a, &empty), 0);

        let identical: Vec<u16> = "আমি বাংলায় গান গাই".encode_utf16().collect();
        assert_eq!(common_prefix_utf16(&identical, &identical), identical.len());
    }
}
