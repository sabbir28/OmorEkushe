#include "LayoutData.h"

#include <algorithm>
#include <cstdio>
#include <windows.h>
#include <shlwapi.h>

#ifdef _MSC_VER
#pragma comment(lib, "shlwapi.lib")
#endif

namespace {

    std::wstring Trim(const std::wstring& value) {
        const size_t start = value.find_first_not_of(L" \t\r\n");
        if (start == std::wstring::npos) {
            return L"";
        }
        const size_t end = value.find_last_not_of(L" \t\r\n");
        return value.substr(start, end - start + 1);
    }

    std::wstring GetTagContent(const std::wstring& line, const wchar_t* tag) {
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
        if (end == std::wstring::npos) return L"";

        return Trim(line.substr(start, end - start));
    }

    bool ParseBool(const std::wstring& value) {
        std::wstring lower = value;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
        return lower == L"true" || lower == L"1" || lower == L"yes";
    }

    int ParseInt(const std::wstring& value) {
        return static_cast<int>(wcstol(value.c_str(), nullptr, 10));
    }

    std::wstring EscapeXml(const std::wstring& text) {
        std::wstring result;
        for (wchar_t c : text) {
            switch (c) {
                case L'<' : result += L"&lt;"; break;
                case L'>' : result += L"&gt;"; break;
                case L'&' : result += L"&amp;"; break;
                case L'\"': result += L"&quot;"; break;
                case L'\'': result += L"&apos;"; break;
                default: result += c; break;
            }
        }
        return result;
    }

    std::wstring UnescapeXml(const std::wstring& text) {
        std::wstring result = text;
        auto replaceAll = [](std::wstring& str, const std::wstring& from, const std::wstring& to) {
            size_t start_pos = 0;
            while((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
        };
        replaceAll(result, L"&lt;", L"<");
        replaceAll(result, L"&gt;", L">");
        replaceAll(result, L"&quot;", L"\"");
        replaceAll(result, L"&apos;", L"'");
        replaceAll(result, L"&amp;", L"&");
        return result;
    }

    std::wstring BoolToStr(bool value) {
        return value ? L"True" : L"False";
    }

} // namespace

namespace editor {

    void LayoutData::clear() {
        key.clear();
        juk.clear();
    }

    bool LayoutData::loadFromFile(const wchar_t* filePath) {
        path = filePath;
        clear();

        FILE* file = _wfopen(filePath, L"r, ccs=UTF-8");
        if (!file) {
            return false;
        }

        wchar_t lineBuffer[2048] = {};
        auto getLine = [&]() -> std::wstring {
            if (!fgetws(lineBuffer, 2048, file)) {
                return L"";
            }
            std::wstring line = lineBuffer;
            if (!line.empty() && line.back() == L'\n') line.pop_back();
            if (!line.empty() && line.back() == L'\r') line.pop_back();
            return Trim(line);
        };

        while (!feof(file)) {
            std::wstring line = getLine();
            if (line.empty()) {
                continue;
            }

            if (line.find(L"<Name>") != std::wstring::npos) {
                name = GetTagContent(line, L"Name");
            } else if (line.find(L"<Shortcut>") != std::wstring::npos) {
                while (!feof(file)) {
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
                mapping.normal = UnescapeXml(GetTagContent(getLine(), L"Normal"));
                mapping.shift = UnescapeXml(GetTagContent(getLine(), L"Shift"));
                key[keyCode] = mapping;
            } else if (line.find(L"<Juk>") != std::wstring::npos) {
                const std::wstring seqValue = GetTagContent(getLine(), L"Seq");
                const std::wstring outValue = GetTagContent(getLine(), L"Out");
                if (!seqValue.empty() || !outValue.empty()) {
                    juk[UnescapeXml(seqValue)] = UnescapeXml(outValue);
                }
            }
        }

        fclose(file);
        return !name.empty();
    }

    bool LayoutData::saveToFile(const wchar_t* filePath) const {
        FILE* file = _wfopen(filePath, L"w, ccs=UTF-8");
        if (!file) {
            return false;
        }

        fwprintf(file, L"<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n");
        fwprintf(file, L"<!--Start of Layout-->\n");
        fwprintf(file, L"<KeyLayout>\n");
        
        fwprintf(file, L"  <Name>%ls</Name>\n", name.c_str());
        
        fwprintf(file, L"  <Shortcut>\n");
        fwprintf(file, L"    <Alt>%ls</Alt>\n", BoolToStr(shortcut.alt).c_str());
        fwprintf(file, L"    <Ctrl>%ls</Ctrl>\n", BoolToStr(shortcut.ctrl).c_str());
        fwprintf(file, L"    <Shift>%ls</Shift>\n", BoolToStr(shortcut.shift).c_str());
        fwprintf(file, L"    <KeyCode>%d</KeyCode>\n", shortcut.keyCode);
        fwprintf(file, L"  </Shortcut>\n");

        if (!iconName.empty()) {
            fwprintf(file, L"  <IconName>%ls</IconName>\n", iconName.c_str());
        } else {
            fwprintf(file, L"  <IconName>%ls</IconName>\n", name.c_str());
        }

        fwprintf(file, L"  <Keys>\n");
        for (const auto& [keyCode, mapping] : key) {
            std::wstring attrs = L"";
            if (!mapping.normalOption.empty()) {
                attrs += L" Normal_Option=\"" + mapping.normalOption + L"\"";
            }
            if (!mapping.shiftOption.empty()) {
                attrs += L" Shift_Option=\"" + mapping.shiftOption + L"\"";
            }
            
            fwprintf(file, L"    <Key KeyCode=\"%d\"%ls>\n", keyCode, attrs.c_str());
            fwprintf(file, L"      <Normal>%ls</Normal>\n", EscapeXml(mapping.normal).c_str());
            fwprintf(file, L"      <Shift>%ls</Shift>\n", EscapeXml(mapping.shift).c_str());
            fwprintf(file, L"    </Key>\n");
        }
        fwprintf(file, L"  </Keys>\n");

        fwprintf(file, L"  <Juks>\n");
        for (const auto& [seq, out] : juk) {
            fwprintf(file, L"    <Juk>\n");
            fwprintf(file, L"      <Seq>%ls</Seq>\n", EscapeXml(seq).c_str());
            fwprintf(file, L"      <Out>%ls</Out>\n", EscapeXml(out).c_str());
            fwprintf(file, L"    </Juk>\n");
        }
        fwprintf(file, L"  </Juks>\n");

        fwprintf(file, L"</KeyLayout>\n");
        fwprintf(file, L"<!--End of Layout-->\n");

        fclose(file);
        return true;
    }

    namespace {

        std::wstring GetExeDirectory() {
            wchar_t path[MAX_PATH] = {};
            if (GetModuleFileNameW(nullptr, path, MAX_PATH) == 0) {
                return L"";
            }
            PathRemoveFileSpecW(path);
            std::wstring p = path;
            if (!p.empty() && p.back() != L'\\') p += L'\\';
            return p;
        }

        void FindXmlRecursive(const std::wstring& dir, std::vector<std::wstring>& out) {
            const std::wstring search = dir + L"*";
            WIN32_FIND_DATAW findData = {};
            HANDLE handle = FindFirstFileW(search.c_str(), &findData);
            if (handle == INVALID_HANDLE_VALUE) {
                return;
            }

            do {
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U) {
                    if (wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0) {
                        FindXmlRecursive(dir + findData.cFileName + L"\\", out);
                    }
                } else if (wcsstr(findData.cFileName, L".xml") != nullptr) {
                    out.push_back(dir + findData.cFileName);
                }
            } while (FindNextFileW(handle, &findData));

            FindClose(handle);
        }

    }

    std::vector<LayoutData> FindLayouts() {
        std::vector<LayoutData> layouts;
        std::wstring appDir = GetExeDirectory();
        
        const std::wstring layoutDirs[] = {
            appDir + L"Layouts\\",
            appDir + L"..\\data\\layout\\",
            appDir + L"..\\data\\Layouts\\"
        };

        std::vector<std::wstring> files;
        for (const auto& layoutDir : layoutDirs) {
            FindXmlRecursive(layoutDir, files);
            if (!files.empty()) {
                break;
            }
        }

        for (size_t i = 0; i < files.size(); ++i) {
            LayoutData layout;
            if (layout.loadFromFile(files[i].c_str())) {
                layout.id = static_cast<int>(i);
                layouts.push_back(std::move(layout));
            }
        }
        return layouts;
    }

} // namespace editor
