#include "KeyboardView.h"
#include "Utf8Utils.h"
#include <vector>

namespace editor {

struct KeyboardKey {
    int keyCode;
    float rx, ry, rw, rh; // Relative coordinates 0.0 to 1.0 (approximated)
    bool modifiable;
    const char* label;
};

// Full standard 104-key US keyboard approximation
static std::vector<KeyboardKey> g_BaseKeys = {
    // Esc & Function Keys (Row 0)
    {27,  0.00f, 0.00f, 0.06f, 0.15f, false, "Esc"},
    {112, 0.10f, 0.00f, 0.06f, 0.15f, false, "F1"}, {113, 0.16f, 0.00f, 0.06f, 0.15f, false, "F2"}, {114, 0.22f, 0.00f, 0.06f, 0.15f, false, "F3"}, {115, 0.28f, 0.00f, 0.06f, 0.15f, false, "F4"},
    {116, 0.36f, 0.00f, 0.06f, 0.15f, false, "F5"}, {117, 0.42f, 0.00f, 0.06f, 0.15f, false, "F6"}, {118, 0.48f, 0.00f, 0.06f, 0.15f, false, "F7"}, {119, 0.54f, 0.00f, 0.06f, 0.15f, false, "F8"},
    {120, 0.62f, 0.00f, 0.06f, 0.15f, false, "F9"}, {121, 0.68f, 0.00f, 0.06f, 0.15f, false, "F10"}, {122, 0.74f, 0.00f, 0.06f, 0.15f, false, "F11"}, {123, 0.80f, 0.00f, 0.06f, 0.15f, false, "F12"},
    {44,  0.87f, 0.00f, 0.05f, 0.15f, false, "Prt"}, {145, 0.93f, 0.00f, 0.05f, 0.15f, false, "Scr"}, {19, 0.99f, 0.00f, 0.05f, 0.15f, false, "Pse"},

    // Number Row (Row 1) (Y offset 0.18)
    {192, 0.00f, 0.18f, 0.05f, 0.15f, true, "`"}, {49,  0.05f, 0.18f, 0.05f, 0.15f, true, "1"}, {50,  0.10f, 0.18f, 0.05f, 0.15f, true, "2"},
    {51,  0.15f, 0.18f, 0.05f, 0.15f, true, "3"}, {52,  0.20f, 0.18f, 0.05f, 0.15f, true, "4"}, {53,  0.25f, 0.18f, 0.05f, 0.15f, true, "5"},
    {54,  0.30f, 0.18f, 0.05f, 0.15f, true, "6"}, {55,  0.35f, 0.18f, 0.05f, 0.15f, true, "7"}, {56,  0.40f, 0.18f, 0.05f, 0.15f, true, "8"},
    {57,  0.45f, 0.18f, 0.05f, 0.15f, true, "9"}, {48,  0.50f, 0.18f, 0.05f, 0.15f, true, "0"}, {189, 0.55f, 0.18f, 0.05f, 0.15f, true, "-"},
    {187, 0.60f, 0.18f, 0.05f, 0.15f, true, "="}, {8,   0.65f, 0.18f, 0.10f, 0.15f, false, "Back"},
    {45,  0.77f, 0.18f, 0.05f, 0.15f, false, "Ins"}, {36,  0.83f, 0.18f, 0.05f, 0.15f, false, "Hm"}, {33, 0.89f, 0.18f, 0.05f, 0.15f, false, "Up"},

    // QWERTY Row (Row 2) (Y offset 0.36)
    {9,   0.00f, 0.36f, 0.08f, 0.15f, false, "Tab"}, {81,  0.08f, 0.36f, 0.05f, 0.15f, true, "Q"}, {87,  0.13f, 0.36f, 0.05f, 0.15f, true, "W"},
    {69,  0.18f, 0.36f, 0.05f, 0.15f, true, "E"}, {82,  0.23f, 0.36f, 0.05f, 0.15f, true, "R"}, {84,  0.28f, 0.36f, 0.05f, 0.15f, true, "T"},
    {89,  0.33f, 0.36f, 0.05f, 0.15f, true, "Y"}, {85,  0.38f, 0.36f, 0.05f, 0.15f, true, "U"}, {73,  0.43f, 0.36f, 0.05f, 0.15f, true, "I"},
    {79,  0.48f, 0.36f, 0.05f, 0.15f, true, "O"}, {80,  0.53f, 0.36f, 0.05f, 0.15f, true, "P"}, {219, 0.58f, 0.36f, 0.05f, 0.15f, true, "["},
    {221, 0.63f, 0.36f, 0.05f, 0.15f, true, "]"}, {220, 0.68f, 0.36f, 0.07f, 0.15f, true, "\\"},
    {46,  0.77f, 0.36f, 0.05f, 0.15f, false, "Del"}, {35,  0.83f, 0.36f, 0.05f, 0.15f, false, "End"}, {34, 0.89f, 0.36f, 0.05f, 0.15f, false, "Dn"},

    // ASDF Row (Row 3) (Y offset 0.54)
    {20,  0.00f, 0.54f, 0.10f, 0.15f, false, "Caps"}, {65,  0.10f, 0.54f, 0.05f, 0.15f, true, "A"}, {83,  0.15f, 0.54f, 0.05f, 0.15f, true, "S"},
    {68,  0.20f, 0.54f, 0.05f, 0.15f, true, "D"}, {70,  0.25f, 0.54f, 0.05f, 0.15f, true, "F"}, {71,  0.30f, 0.54f, 0.05f, 0.15f, true, "G"},
    {72,  0.35f, 0.54f, 0.05f, 0.15f, true, "H"}, {74,  0.40f, 0.54f, 0.05f, 0.15f, true, "J"}, {75,  0.45f, 0.54f, 0.05f, 0.15f, true, "K"},
    {76,  0.50f, 0.54f, 0.05f, 0.15f, true, "L"}, {186, 0.55f, 0.54f, 0.05f, 0.15f, true, ";"}, {222, 0.60f, 0.54f, 0.05f, 0.15f, true, "'"},
    {13,  0.65f, 0.54f, 0.10f, 0.15f, false, "Enter"},

    // ZXCV Row (Row 4) (Y offset 0.72)
    {16,  0.00f, 0.72f, 0.13f, 0.15f, false, "Shift"}, {90,  0.13f, 0.72f, 0.05f, 0.15f, true, "Z"}, {88,  0.18f, 0.72f, 0.05f, 0.15f, true, "X"},
    {67,  0.23f, 0.72f, 0.05f, 0.15f, true, "C"}, {86,  0.28f, 0.72f, 0.05f, 0.15f, true, "V"}, {66,  0.33f, 0.72f, 0.05f, 0.15f, true, "B"},
    {78,  0.38f, 0.72f, 0.05f, 0.15f, true, "N"}, {77,  0.43f, 0.72f, 0.05f, 0.15f, true, "M"}, {188, 0.48f, 0.72f, 0.05f, 0.15f, true, ","},
    {190, 0.53f, 0.72f, 0.05f, 0.15f, true, "."}, {191, 0.58f, 0.72f, 0.05f, 0.15f, true, "/"}, {16, 0.63f, 0.72f, 0.12f, 0.15f, false, "Shift"},
    {38,  0.83f, 0.72f, 0.05f, 0.15f, false, "Up"},

    // Space Row (Row 5) (Y offset 0.90)
    {17,  0.00f, 0.90f, 0.06f, 0.10f, false, "Ctrl"}, {91,  0.06f, 0.90f, 0.06f, 0.10f, false, "Win"}, {18,  0.12f, 0.90f, 0.06f, 0.10f, false, "Alt"},
    {32,  0.18f, 0.90f, 0.39f, 0.10f, false, "Space"}, {18,  0.57f, 0.90f, 0.06f, 0.10f, false, "Alt"}, {92,  0.63f, 0.90f, 0.06f, 0.10f, false, "Win"},
    {93,  0.69f, 0.90f, 0.06f, 0.10f, false, "Menu"}, {17,  0.75f, 0.90f, 0.06f, 0.10f, false, "Ctrl"},
    {37,  0.77f, 0.90f, 0.05f, 0.10f, false, "Left"}, {40,  0.83f, 0.90f, 0.05f, 0.10f, false, "Dn"}, {39, 0.89f, 0.90f, 0.05f, 0.10f, false, "Rt"}
};

bool KeyboardView::Render(const LayoutData& layout, ImVec2 size, int& clickedKeyCode) {
    bool clicked = false;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);

    ImGui::InvisibleButton("##KeyboardCanvas", size);
    bool is_hovered = ImGui::IsItemHovered();
    ImVec2 mouse_pos = ImGui::GetIO().MousePos;

    // Draw background
    draw_list->AddRectFilled(p0, p1, IM_COL32(245, 247, 250, 255));

    for (const auto& k : g_BaseKeys) {
        float kx = p0.x + k.rx * size.x;
        float ky = p0.y + k.ry * size.y;
        float kw = k.rw * size.x;
        float kh = k.rh * size.y;

        ImVec2 kp0 = ImVec2(kx + 2, ky + 2);
        ImVec2 kp1 = ImVec2(kx + kw - 2, ky + kh - 2);

        bool hovered = is_hovered && mouse_pos.x >= kp0.x && mouse_pos.x <= kp1.x && mouse_pos.y >= kp0.y && mouse_pos.y <= kp1.y;
        if (hovered && ImGui::IsMouseClicked(0) && k.modifiable) {
            clickedKeyCode = k.keyCode;
            clicked = true;
        }

        // Depth/Shadow (subtle bottom edge)
        draw_list->AddRectFilled(ImVec2(kp0.x, kp1.y - 4), kp1, IM_COL32(200, 205, 215, 255), 8.0f, ImDrawFlags_RoundCornersBottom);
        
        // Key Body
        ImU32 col = k.modifiable ? IM_COL32(255, 255, 255, 255) : IM_COL32(240, 243, 249, 255);
        if (hovered && k.modifiable) col = IM_COL32(235, 245, 255, 255);
        
        draw_list->AddRectFilled(kp0, kp1, col, 8.0f);
        draw_list->AddRect(kp0, kp1, IM_COL32(190, 200, 210, 255), 8.0f, 0, 1.5f);

        // Labels
        if (k.modifiable) {
            // Base label (very faint original US key)
            draw_list->AddText(ImVec2(kp0.x + 8, kp1.y - 18), IM_COL32(220, 225, 235, 255), k.label);

            auto it = layout.key.find(k.keyCode);
            if (it != layout.key.end()) {
                std::string normal = WStringToUtf8(it->second.normal);
                std::string shift = WStringToUtf8(it->second.shift);

                if (!normal.size() == 0) {
                    // Center the main character a bit more or put it at bottom-right
                    ImVec2 textSize = ImGui::CalcTextSize(normal.c_str());
                    draw_list->AddText(ImVec2(kp1.x - textSize.x - 8, kp1.y - textSize.y - 8), IM_COL32(45, 55, 72, 255), normal.c_str());
                }
                if (!shift.size() == 0) {
                    // Put shift character at top-left
                    draw_list->AddText(ImVec2(kp0.x + 8, kp0.y + 6), IM_COL32(100, 120, 140, 255), shift.c_str());
                }
            }
        } else {
            ImVec2 textSize = ImGui::CalcTextSize(k.label);
            draw_list->AddText(ImVec2(kp0.x + (kw - textSize.x) / 2, kp0.y + (kh - textSize.y) / 2), IM_COL32(113, 128, 150, 255), k.label);
        }
    }

    return clicked;
}

} // namespace editor
