#pragma once

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/// SIMD & Assembly-accelerated UTF-16 common prefix length scanner.
size_t omorekushe_fast_simd_prefix(
    const uint16_t* s1, size_t len1,
    const uint16_t* s2, size_t len2);

/// Fast diff computation for dynamic phonetic backspacing.
/// Returns commonPrefixLength, and outputs out_common and out_backspaces.
size_t omorekushe_fast_compute_diff(
    const uint16_t* old_buf, size_t old_len,
    const uint16_t* new_buf, size_t new_len,
    size_t* out_common,
    size_t* out_backspaces);

/// High-speed zero-allocation Avro case normalization.
size_t omorekushe_fast_fix_string(
    const uint8_t* input, size_t len,
    uint8_t* out_buf, size_t out_capacity);

/// Single-cycle character classifications
bool omorekushe_fast_is_vowel(uint8_t c);
bool omorekushe_fast_is_consonant(uint8_t c);

#ifdef __cplusplus
}
#endif

