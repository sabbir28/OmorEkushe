// =============================================================================
// File   : layout.cpp
// Module : core (Keyboard Layout Parser)
// Purpose: Implements the XML-based keyboard layout file parser. Each layout
//          XML file defines a named keyboard mapping (e.g. "Bijoy", "National")
//          with the following structure:
//
//            <Name>LayoutName</Name>
//            <Shortcut><Ctrl>true</Ctrl><Alt>true</Alt><KeyCode>66</KeyCode></Shortcut>
//            <IconName>icon.ico</IconName>
//            <Key KeyCode="65" Normal_Option="..." Shift_Option="...">
//              <Normal>অ</Normal>
//              <Shift>ঔ</Shift>
//            </Key>
//            <Juk><Seq>...</Seq><Out>...</Out></Juk>
//
//          The parser is a lightweight custom XML scanner (not a full DOM parser)
//          that processes the file line-by-line for maximum robustness against
//          malformed or missing elements.
//
//          All string parsing functions are exception-safe with catch-all guards.
// =============================================================================

#include "core/layout.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <windows.h>

namespace {

    std::wstring Trim(const std::wstring& value) {
        try {
            const size_t start = value.find_first_not_of(L" \t\r\n");
            if (start == std::wstring::npos) {
                return L"";
            }
            const size_t end = value.find_last_not_of(L" \t\r\n");
            if (end == std::wstring::npos || end < start || start > value.size()) {
                return L"";
            }
            return value.substr(start, end - start + 1);
        } catch (...) {
            return L"";
        }
    }

    std::wstring GetTagContent(const std::wstring& line, const wchar_t* tag) {
        try {
            std::wstring open = L"<";
            open += tag;
            open += L">";
            std::wstring close = L"</";
            close += tag;
            close += L">";

            size_t start = line.find(open);
            if (start == std::wstring::npos) return L"";
            start += open.size();

            const size_t end = line.find(close, start);
            if (end == std::wstring::npos || end < start || start > line.size()) return L"";

            return Trim(line.substr(start, end - start));
        } catch (...) {
            return L"";
        }
    }

    bool ParseBool(const std::wstring& value) {
        std::wstring lower = value;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
        return lower == L"true" || lower == L"1" || lower == L"yes";
    }

    int ParseInt(const std::wstring& value) {
        return static_cast<int>(wcstol(value.c_str(), nullptr, 10));
    }

} // namespace

namespace bijoy::core {

    void Layout::clear() {
        key.clear();
        juk.clear();
    }

    bool Layout::loadFromFile(const wchar_t* filePath) {
        if (!filePath) return false;
        path = filePath;
        clear();

        std::unique_ptr<FILE, decltype(&fclose)> file(_wfopen(filePath, L"r, ccs=UTF-8"), &fclose);
        if (!file) {
            return false;
        }

        wchar_t lineBuffer[2048] = {};
        auto getLine = [&]() -> std::wstring {
            if (!fgetws(lineBuffer, 2048, file.get())) {
                return L"";
            }
            std::wstring line = lineBuffer;
            if (!line.empty() && line.back() == L'\n') line.pop_back();
            if (!line.empty() && line.back() == L'\r') line.pop_back();
            return Trim(line);
        };

        try {
            while (!feof(file.get())) {
                std::wstring line = getLine();
                if (line.empty()) {
                    continue;
                }

                if (line.find(L"<Name>") != std::wstring::npos) {
                    name = GetTagContent(line, L"Name");
                } else if (line.find(L"<Shortcut>") != std::wstring::npos) {
                    while (!feof(file.get())) {
                        std::wstring sub = getLine();
                        if (sub.find(L"</Shortcut>") != std::wstring::npos) break;
                        if (sub.find(L"<Alt>") != std::wstring::npos) shortcut.alt = ParseBool(GetTagContent(sub, L"Alt"));
                        else if (sub.find(L"<Ctrl>") != std::wstring::npos) shortcut.ctrl = ParseBool(GetTagContent(sub, L"Ctrl"));
                        else if (sub.find(L"<Shift>") != std::wstring::npos) shortcut.shift = ParseBool(GetTagContent(sub, L"Shift"));
                        else if (sub.find(L"<KeyCode>") != std::wstring::npos) shortcut.keyCode = ParseInt(GetTagContent(sub, L"KeyCode"));
                    }
                } else if (line.find(L"<IconName>") != std::wstring::npos) {
                    iconName = GetTagContent(line, L"IconName");
                } else if (line.find(L"<Key ") != std::wstring::npos) {
                    int keyCode = 0;
                    std::wstring normalOpt;
                    std::wstring shiftOpt;

                    size_t kc = line.find(L"KeyCode=\"");
                    if (kc != std::wstring::npos) {
                        kc += 9;
                        size_t end = line.find(L"\"", kc);
                        if (end != std::wstring::npos) keyCode = ParseInt(line.substr(kc, end - kc));
                    }

                    size_t no = line.find(L"Normal_Option=\"");
                    if (no != std::wstring::npos) {
                        no += 15;
                        size_t end = line.find(L"\"", no);
                        if (end != std::wstring::npos) normalOpt = line.substr(no, end - no);
                    }

                    size_t so = line.find(L"Shift_Option=\"");
                    if (so != std::wstring::npos) {
                        so += 14;
                        size_t end = line.find(L"\"", so);
                        if (end != std::wstring::npos) shiftOpt = line.substr(so, end - so);
                    }

                    KeyMapping mapping;
                    mapping.normalOption = normalOpt;
                    mapping.shiftOption = shiftOpt;
                    mapping.normal = GetTagContent(getLine(), L"Normal");
                    mapping.shift = GetTagContent(getLine(), L"Shift");
                    key[keyCode] = mapping;
                } else if (line.find(L"<Juk>") != std::wstring::npos) {
                    const std::wstring seqValue = GetTagContent(getLine(), L"Seq");
                    const std::wstring outValue = GetTagContent(getLine(), L"Out");
                    if (!seqValue.empty() || !outValue.empty()) {
                        juk[seqValue] = outValue;
                    }
                }
            }
        } catch (...) {
            return false;
        }

        return !name.empty();
    }

    bool Layout::updateFromFile() {
        return loadFromFile(path.c_str());
    }

} // namespace bijoy::core
