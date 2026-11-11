//! Zero-allocation phonetic parsing and classification engine.

/// Compile-time 256-byte character classification and case-normalization lookup table.
pub const CASE_MAP: [u8; 256] = {
    let mut map = [0u8; 256];
    let mut i = 0usize;
    while i < 256 {
        let b = i as u8;
        let lower = if b >= b'A' && b <= b'Z' { b + 32 } else { b };
        // Avro case-sensitive letters: o, i, u, d, g, j, n, r, s, t, y, z
        let is_case_sensitive = matches!(
            lower,
            b'o' | b'i' | b'u' | b'd' | b'g' | b'j' | b'n' | b'r' | b's' | b't' | b'y' | b'z'
        );
        map[i] = if is_case_sensitive { b } else { lower };
        i += 1;
    }
    map
};

const FLAG_VOWEL: u8 = 1 << 0;
const FLAG_CONSONANT: u8 = 1 << 1;
const FLAG_NUMBER: u8 = 1 << 2;
const FLAG_PUNCTUATION: u8 = 1 << 3;

const CHAR_PROPERTIES: [u8; 256] = {
    let mut props = [0u8; 256];
    let mut i = 0usize;
    while i < 256 {
        let b = i as u8;
        let lower = if b >= b'A' && b <= b'Z' { b + 32 } else { b };
        let is_vowel = matches!(lower, b'a' | b'e' | b'i' | b'o' | b'u');
        let is_consonant = (lower >= b'a' && lower <= b'z') && !is_vowel;
        let is_number = b >= b'0' && b <= b'9';
        let is_punct = !is_vowel && !is_consonant;

        let mut flag = 0u8;
        if is_vowel { flag |= FLAG_VOWEL; }
        if is_consonant { flag |= FLAG_CONSONANT; }
        if is_number { flag |= FLAG_NUMBER; }
        if is_punct { flag |= FLAG_PUNCTUATION; }

        props[i] = flag;
        i += 1;
    }
    props
};

#[inline(always)]
pub fn is_vowel(c: u8) -> bool {
    (CHAR_PROPERTIES[c as usize] & FLAG_VOWEL) != 0
}

#[inline(always)]
pub fn is_consonant(c: u8) -> bool {
    (CHAR_PROPERTIES[c as usize] & FLAG_CONSONANT) != 0
}

#[inline(always)]
pub fn is_number(c: u8) -> bool {
    (CHAR_PROPERTIES[c as usize] & FLAG_NUMBER) != 0
}

#[inline(always)]
pub fn is_punctuation(c: u8) -> bool {
    (CHAR_PROPERTIES[c as usize] & FLAG_PUNCTUATION) != 0
}

/// Normalizes ASCII input characters according to Avro phonetic case rules in-place
/// without heap allocations.
#[inline]
pub fn fix_string_in_place(buf: &mut [u8]) {
    for b in buf.iter_mut() {
        *b = CASE_MAP[*b as usize];
    }
}

/// Normalizes input string into a destination buffer.
#[inline]
pub fn fix_string(input: &str, output: &mut String) {
    output.clear();
    output.reserve(input.len());
    for &b in input.as_bytes() {
        output.push(CASE_MAP[b as usize] as char);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_classification() {
        assert!(is_vowel(b'a'));
        assert!(is_vowel(b'A'));
        assert!(!is_vowel(b'b'));
        assert!(is_consonant(b'B'));
        assert!(is_consonant(b'k'));
        assert!(is_number(b'5'));
        assert!(is_punctuation(b'.'));
        assert!(is_punctuation(b' '));
    }

    #[test]
    fn test_fix_string() {
        let mut out = String::new();
        fix_string("ODEr AMAr", &mut out);
        assert_eq!(out, "ODer amar");
    }
}
