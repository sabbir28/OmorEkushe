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

/// Bit-parallel Myers Levenshtein distance using assembly POPCNT.
/// Executes in nanoseconds with zero matrix/vector dynamic heap allocations.
uint32_t omorekushe_fast_levenshtein(
    const uint32_t* a, size_t len_a,
    const uint32_t* b, size_t len_b,
    uint32_t max_dist);

/// Zero-allocation UTF-8 to Unicode codepoints decoder.
size_t omorekushe_fast_utf8_to_codepoints(
    const uint8_t* utf8_ptr, size_t utf8_len,
    uint32_t* out_cp, size_t max_out);

/// Smart macro expander (date, time, currency amounts like 1500/=, and shorthands like //om).
/// Writes expanded UTF-16 code units into out_utf16 and returns the number of code units.
/// Returns 0 if no macro matched.
size_t omorekushe_fast_expand_macro(
    const char* raw_ascii, size_t raw_len,
    uint16_t* out_utf16, size_t max_out);

/// Predictive autocomplete candidate search.
/// Matches prefix_utf16 against the high-frequency Bengali vocabulary.
/// Populates out_buffer with null-terminated UTF-16 candidate strings and out_offsets with character offsets.
/// Returns the number of candidate words found (up to max_candidates).
size_t omorekushe_fast_predict_candidates(
    const uint16_t* prefix_utf16, size_t prefix_len,
    uint16_t* out_buffer, size_t max_out_chars,
    size_t* out_offsets, size_t max_candidates);

#ifdef __cplusplus
}
#endif

