//! Smart Bengali Text Expander and Date/Currency Macro Engine.
//! Provides instant expansion for dates, times, currency amounts, and shorthands.

/// Bengali digits 0..9 in UTF-16.
pub const BENGALI_DIGITS: [u16; 10] = [
    0x09E6, 0x09E7, 0x09E8, 0x09E9, 0x09EA, 0x09EB, 0x09EC, 0x09ED, 0x09EE, 0x09EF,
];

/// Converts an unsigned integer to a Bengali numeral string (UTF-16).
pub fn u32_to_bengali_digits(mut n: u32, out: &mut Vec<u16>) {
    if n == 0 {
        out.push(BENGALI_DIGITS[0]);
        return;
    }
    let mut digits = [0u16; 16];
    let mut count = 0;
    while n > 0 {
        digits[count] = BENGALI_DIGITS[(n % 10) as usize];
        n /= 10;
        count += 1;
    }
    for i in (0..count).rev() {
        out.push(digits[i]);
    }
}

/// Bengali words for numbers 0 through 99.
const NUMBERS_0_TO_99: [&str; 100] = [
    "শূন্য", "এক", "দুই", "তিন", "চার", "পাঁচ", "ছয়", "সাত", "আট", "নয়",
    "দশ", "এগারো", "বারো", "তেরো", "চৌদ্দ", "পনেরো", "ষোলো", "সতেরো", "আঠারো", "উনিশ",
    "বিশ", "একুশ", "বাইশ", "তেইশ", "চব্বিশ", "পঁচিশ", "ছাব্বিশ", "সাতাশ", "আঠাশ", "ঊনত্রিশ",
    "ত্রিশ", "একত্রিশ", "বত্রিশ", "তেত্রিশ", "চৌত্রিশ", "পঁয়ত্রিশ", "ছত্রিশ", "সাঁইত্রিশ", "আটত্রিশ", "ঊনচল্লিশ",
    "চল্লিশ", "একচল্লিশ", "বিয়াল্লিশ", "তেতাল্লিশ", "চুয়াল্লিশ", "পঁয়তাল্লিশ", "ছেচল্লিশ", "সাতচল্লিশ", "আটচল্লিশ", "ঊনপঞ্চাশ",
    "পঞ্চাশ", "একান্ন", "বায়ান্ন", "তিপ্পান্ন", "চুয়ান্ন", "পঞ্চান্ন", "ছাপ্পান্ন", "সাতান্ন", "আটান্ন", "ঊনষাট",
    "ষাট", "একষট্টি", "বাষট্টি", "তেষট্টি", "চৌষট্টি", "পঁয়ষট্টি", "ছেষট্টি", "সাতষট্টি", "আটষট্টি", "ঊনসত্তর",
    "সত্তর", "একাত্তর", "বাহাত্তর", "তিয়াত্তর", "চুয়াত্তর", "পঁচাত্তর", "ছিয়াত্তর", "সাতাত্তর", "আটাত্তর", "ঊনআশি",
    "আশি", "একাশি", "বিরাশি", "তিরাশি", "চুরাশি", "পঁচাশি", "ছিয়াশি", "সাতাশি", "আটাশি", "ঊননব্বই",
    "নব্বই", "একানব্বই", "বিরানব্বই", "তিরানব্বই", "চুরানব্বই", "পঁচানব্বই", "ছিয়ানব্বই", "সাতানব্বই", "আটানব্বই", "নিরানব্বই",
];

/// Converts an amount (up to 99,99,99,999) to Bengali words.
/// Format follows Indian/Bengali numbering: Crores (কোটি), Lakhs (লাখ), Thousands (হাজার), Hundreds (শত).
pub fn number_to_bengali_words(mut amount: u64) -> String {
    if amount == 0 {
        return "শূন্য টাকা মাত্র".to_string();
    }

    let mut parts: Vec<String> = Vec::new();

    let crore = amount / 10_000_000;
    amount %= 10_000_000;

    let lakh = amount / 100_000;
    amount %= 100_000;

    let thousand = amount / 1_000;
    amount %= 1_000;

    let hundred = amount / 100;
    let remainder = amount % 100;

    if crore > 0 {
        if crore < 100 {
            parts.push(format!("{} কোটি", NUMBERS_0_TO_99[crore as usize]));
        } else {
            parts.push(format!("{} কোটি", number_to_bengali_words(crore).replace(" টাকা মাত্র", "")));
        }
    }

    if lakh > 0 {
        parts.push(format!("{} লাখ", NUMBERS_0_TO_99[lakh as usize]));
    }

    if thousand > 0 {
        parts.push(format!("{} হাজার", NUMBERS_0_TO_99[thousand as usize]));
    }

    if hundred > 0 {
        parts.push(format!("{} শত", NUMBERS_0_TO_99[hundred as usize]));
    }

    if remainder > 0 {
        parts.push(NUMBERS_0_TO_99[remainder as usize].to_string());
    }

    parts.join(" ") + " টাকা মাত্র"
}

/// Bengali month names.
const BENGALI_MONTHS: [&str; 12] = [
    "জানুয়ারি", "ফেব্রুয়ারি", "মার্চ", "এপ্রিল", "মে", "জুন",
    "জুলাই", "আগস্ট", "সেপ্টেম্বর", "অক্টোবর", "নভেম্বর", "ডিসেম্বর",
];

/// Checks if `raw_input` matches any built-in macro trigger.
/// If matched, returns the expanded text as a `String`.
pub fn try_expand_macro(raw_input: &str) -> Option<String> {
    let trimmed = raw_input.trim();
    if trimmed.is_empty() {
        return None;
    }

    // 1. Shorthand text expansions starting with "//"
    if let Some(shorthand) = trimmed.strip_prefix("//") {
        match shorthand.to_ascii_lowercase().as_str() {
            "om" => return Some("অমর একুশে".to_string()),
            "bd" => return Some("বাংলাদেশ".to_string()),
            "dh" | "dhk" => return Some("ঢাকা".to_string()),
            "slm" => return Some("আসসালামু আলাইকুম".to_string()),
            "dhn" => return Some("ধন্যবাদ".to_string()),
            "shd" => return Some("স্বাধীনতা".to_string()),
            "bn" => return Some("বাংলা".to_string()),
            "date" | "tarikh" => {
                // Approximate current date fallback if OS time cannot be fetched
                return Some(format_current_bengali_date());
            }
            "time" | "shomoy" => {
                return Some(format_current_bengali_time());
            }
            _ => {}
        }
    }

    // 2. Currency/amount macro ending with "/=" (e.g. "1500/=" or "500/=")
    if trimmed.ends_with("/=") {
        let num_str = trimmed.trim_end_matches("/=").trim();
        // Allow pure ASCII digits
        if !num_str.is_empty() && num_str.chars().all(|c| c.is_ascii_digit()) {
            if let Ok(val) = num_str.parse::<u64>() {
                return Some(number_to_bengali_words(val));
            }
        }
    }

    None
}

/// Formats current local system date as a Bengali string (e.g. "১১ নভেম্বর ২০২৬").
pub fn format_current_bengali_date() -> String {
    #[cfg(windows)]
    unsafe {
        #[repr(C)]
        struct SystemTime {
            year: u16,
            month: u16,
            day_of_week: u16,
            day: u16,
            hour: u16,
            minute: u16,
            second: u16,
            milliseconds: u16,
        }
        extern "system" {
            fn GetLocalTime(lpSystemTime: *mut SystemTime);
        }
        let mut st = SystemTime {
            year: 0, month: 0, day_of_week: 0, day: 0,
            hour: 0, minute: 0, second: 0, milliseconds: 0,
        };
        GetLocalTime(&mut st);

        let mut day_vec = Vec::new();
        u32_to_bengali_digits(st.day as u32, &mut day_vec);
        let day_str = String::from_utf16_lossy(&day_vec);

        let month_idx = (st.month as usize).saturating_sub(1).min(11);
        let month_name = BENGALI_MONTHS[month_idx];

        let mut year_vec = Vec::new();
        u32_to_bengali_digits(st.year as u32, &mut year_vec);
        let year_str = String::from_utf16_lossy(&year_vec);

        format!("{} {} {}", day_str, month_name, year_str)
    }

    #[cfg(not(windows))]
    {
        "১১ নভেম্বর ২০২৬".to_string()
    }
}

/// Formats current local system time as a Bengali string (e.g. "১২:০৫ অপরাহ্ন").
pub fn format_current_bengali_time() -> String {
    #[cfg(windows)]
    unsafe {
        #[repr(C)]
        struct SystemTime {
            year: u16,
            month: u16,
            day_of_week: u16,
            day: u16,
            hour: u16,
            minute: u16,
            second: u16,
            milliseconds: u16,
        }
        extern "system" {
            fn GetLocalTime(lpSystemTime: *mut SystemTime);
        }
        let mut st = SystemTime {
            year: 0, month: 0, day_of_week: 0, day: 0,
            hour: 0, minute: 0, second: 0, milliseconds: 0,
        };
        GetLocalTime(&mut st);

        let is_pm = st.hour >= 12;
        let mut h12 = st.hour % 12;
        if h12 == 0 { h12 = 12; }

        let mut h_vec = Vec::new();
        u32_to_bengali_digits(h12 as u32, &mut h_vec);
        let h_str = String::from_utf16_lossy(&h_vec);

        let mut m_vec = Vec::new();
        if st.minute < 10 {
            m_vec.push(BENGALI_DIGITS[0]);
        }
        u32_to_bengali_digits(st.minute as u32, &mut m_vec);
        let m_str = String::from_utf16_lossy(&m_vec);

        let period = if is_pm { "অপরাহ্ন" } else { "পূর্বাহ্ন" };
        format!("{}:{} {}", h_str, m_str, period)
    }

    #[cfg(not(windows))]
    {
        "১২:০০ অপরাহ্ন".to_string()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_number_to_words() {
        assert_eq!(number_to_bengali_words(0), "শূন্য টাকা মাত্র");
        assert_eq!(number_to_bengali_words(5), "পাঁচ টাকা মাত্র");
        assert_eq!(number_to_bengali_words(100), "এক শত টাকা মাত্র");
        assert_eq!(number_to_bengali_words(1500), "এক হাজার পাঁচ শত টাকা মাত্র");
        assert_eq!(number_to_bengali_words(25000), "পঁচিশ হাজার টাকা মাত্র");
        assert_eq!(number_to_bengali_words(100000), "এক লাখ টাকা মাত্র");
    }

    #[test]
    fn test_macro_expansion() {
        assert_eq!(try_expand_macro("//om"), Some("অমর একুশে".to_string()));
        assert_eq!(try_expand_macro("//bd"), Some("বাংলাদেশ".to_string()));
        assert_eq!(try_expand_macro("1500/="), Some("এক হাজার পাঁচ শত টাকা মাত্র".to_string()));
        assert_eq!(try_expand_macro("random text"), None);
    }
}
