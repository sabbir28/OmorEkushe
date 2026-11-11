//! Predictive Next-Word Suggestion Engine.
//! Provides ultra-low latency candidate completions for active Bengali prefixes.

/// High-frequency curated Bengali vocabulary for instant prefix matching.
pub const BENGALI_COMMON_VOCAB: &[&str] = &[
    "অমর", "একুশে", "অনেক", "অনুগ্রহ", "অন্যান্য", "অবশ্যই", "অর্থ", "অর্থনৈতিক",
    "অফিস", "অপেক্ষা", "অবস্থা", "আইন", "আজ", "আজকে", "আছে", "আছেন", "আছি", "আছো",
    "আন্তর্জাতিক", "আবার", "আমরা", "আমাদের", "আমাকে", "আমার", "আমি", "আপনি",
    "আপনার", "আপনাদের", "আসসালামু", "আলাইকুম", "আসুন", "আসা", "আগে", "আন্দোলন",
    "ইচ্ছা", "ইতিহাস", "ইন্টারনেট", "উত্তর", "উপকার", "উপস্থিত", "উপলক্ষে", "উন্নয়ন",
    "এক", "একটু", "একটি", "একমাত্র", "একসাথে", "এখানে", "এখন", "এখনও",
    "এবং", "ঐতিহ্য", "ওপর", "কথা", "করতে", "করবেন", "করুন", "করে", "করা", "করছি",
    "করব", "কম্পিউটার", "কখনও", "কখন", "কখনো", "কীভাবে", "কী", "কারণ",
    "কাজ", "কাছে", "কাউকে", "কেমন", "কোনো", "কোথায়", "খুব", "খবর", "খেলা",
    "খোলা", "গল্প", "গান", "গ্রাম", "ঘটনা", "ঘর", "চলুন", "চাওয়া", "চিত্র",
    "চিন্তা", "ছোট", "ছবি", "জনগণ", "জন্ম", "জরুরি", "জাতীয়", "জীবন", "জানতে",
    "জানা", "জ্ঞান", "ঠিক", "তথ্য", "তারিখ", "তারা", "তাদের", "তিনি", "তুমি",
    "তোমাদের", "তোমার", "তুমি", "তোমাকে", "তখন", "তারপর", "দিন", "দৃষ্টি",
    "দেশ", "দেশের", "দেহ", "দেখা", "দিতে", "দয়া", "ধন্যবাদ", "ধর্ম",
    "নতুন", "নম্বর", "নদী", "না", "নাম", "নিয়ম", "নিচে", "নিশ্চয়",
    "পরবর্তী", "পরিবর্তন", "পরিবার", "পরীক্ষা", "পড়া", "পাঠ", "পানি", "পাওয়া",
    "পছন্দ", "প্রথম", "প্রধান", "প্রয়োজন", "প্রশ্ন", "প্রযুক্তি", "প্রকাশ",
    "ফোন", "ফলাফল", "ফেরত", "বাংলা", "বাঙালি", "বাঙালী", "বাংলাদেশ", "বাংলাদেশের",
    "বন্ধু", "বছর", "ব্যবস্থা", "ব্যবসা", "ভালো", "ভালোবাসা", "ভালোভাবে", "ভাষা",
    "মানুষ", "মনে", "মন", "মাঝে", "মাধ্যম", "মাস", "মতামত", "মুক্তিযুদ্ধ",
    "যখন", "যদি", "যাচ্ছে", "যাওয়া", "যোগাযোগ", "যুদ্ধ", "রক্ত", "রাত",
    "রাজনীতি", "রাস্তা", "রূপ", "লেখা", "লাখ", "লোক", "শিক্ষা", "শিক্ষক",
    "শিশু", "শহিদ", "শহর", "শান্তি", "শব্দ", "সুন্দর", "সুন্দরী", "সুবিধা",
    "সময়", "সবাই", "সবাইকে", "সবসময়", "সবকিছু", "সহায়তা", "সহজ", "সরাসরি",
    "সমাজ", "সরকার", "সফল", "সংবাদ", "সম্পর্ক", "সাহায্য", "স্বাধীনতা", "স্বাধীন",
    "হবে", "হলে", "হওয়ার", "হয়ে", "হয়েছে", "হিসেবে", "হৃদয়",
];

/// Encodes a UTF-8 string into a preallocated slice of UTF-16 code units.
/// Returns number of UTF-16 units written.
pub fn str_to_utf16_slice(s: &str, out: &mut [u16]) -> usize {
    let mut count = 0;
    for u in s.encode_utf16() {
        if count >= out.len() {
            break;
        }
        out[count] = u;
        count += 1;
    }
    count
}

/// Finds predictive candidate words that start with `prefix_utf16`.
/// Returns the number of candidates found up to `max_candidates`.
/// Candidates are stored in `out_buffer` as null-terminated UTF-16 strings,
/// with character offsets stored in `out_offsets`.
pub fn predict_candidates_utf16(
    prefix_utf16: &[u16],
    out_buffer: &mut [u16],
    out_offsets: &mut [usize],
    max_candidates: usize,
) -> usize {
    if prefix_utf16.is_empty() || max_candidates == 0 || out_buffer.is_empty() {
        return 0;
    }

    let mut found = 0;
    let mut buf_pos = 0;
    let max_buf = out_buffer.len();

    // Reusable word buffer for comparisons
    let mut word_utf16 = [0u16; 64];

    for &word in BENGALI_COMMON_VOCAB {
        if found >= max_candidates {
            break;
        }

        let word_len = str_to_utf16_slice(word, &mut word_utf16);
        if word_len >= prefix_utf16.len() {
            // Check prefix match
            if &word_utf16[..prefix_utf16.len()] == prefix_utf16 {
                // Check if buffer has room for word + null terminator
                if buf_pos + word_len + 1 <= max_buf {
                    out_offsets[found] = buf_pos;
                    out_buffer[buf_pos..buf_pos + word_len].copy_from_slice(&word_utf16[..word_len]);
                    out_buffer[buf_pos + word_len] = 0; // Null-terminator
                    buf_pos += word_len + 1;
                    found += 1;
                }
            }
        }
    }

    found
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_predict_candidates() {
        let prefix: Vec<u16> = "আম".encode_utf16().collect();
        let mut out_buffer = [0u16; 256];
        let mut out_offsets = [0usize; 10];

        let count = predict_candidates_utf16(&prefix, &mut out_buffer, &mut out_offsets, 5);
        assert!(count >= 3);

        // Verify first suggestion starts with "আম"
        let first_offset = out_offsets[0];
        let first_len = out_buffer[first_offset..].iter().position(|&c| c == 0).unwrap();
        let first_str = String::from_utf16_lossy(&out_buffer[first_offset..first_offset + first_len]);
        assert!(first_str.starts_with("আম"));
    }

    #[test]
    fn test_predict_bangla() {
        let prefix: Vec<u16> = "বাং".encode_utf16().collect();
        let mut out_buffer = [0u16; 256];
        let mut out_offsets = [0usize; 10];

        let count = predict_candidates_utf16(&prefix, &mut out_buffer, &mut out_offsets, 5);
        assert!(count >= 2);
    }
}
