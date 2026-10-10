#include "KeyboardView.h"
#include "Utf8Utils.h"
#include <vector>
#include <string>
#include <algorithm>

namespace editor {

ImFont* g_FontUI = nullptr;
ImFont* g_FontKeyLarge = nullptr;
ImFont* g_FontKeySub = nullptr;
ImFont* g_FontKeySmall = nullptr;
bool g_DarkMode = false; // Default to clean, high-contrast light keyboard view

struct KeyboardKey {
    int keyCode;
    float ux, uy, uw, uh; // Unit coordinates (1.0u = standard 1u key)
    bool modifiable;
    const char* label;     // Display / base physical key label
};

// Mathematically exact ANSI 87-key Tenkeyless (TKL) layout
// Typing block is 15.0u wide across all 6 rows
// Navigation cluster is 3.0u wide, starting at 15.5u, ending at 18.5u
static const std::vector<KeyboardKey> g_BaseKeys = {
    // Row 0: Function Keys (y = 0.00u, h = 0.85u)
    {27,  0.00f, 0.00f, 1.00f, 0.85f, false, "Esc"},
    {112, 2.00f, 0.00f, 1.00f, 0.85f, false, "F1"},
    {113, 3.00f, 0.00f, 1.00f, 0.85f, false, "F2"},
    {114, 4.00f, 0.00f, 1.00f, 0.85f, false, "F3"},
    {115, 5.00f, 0.00f, 1.00f, 0.85f, false, "F4"},
    {116, 6.50f, 0.00f, 1.00f, 0.85f, false, "F5"},
    {117, 7.50f, 0.00f, 1.00f, 0.85f, false, "F6"},
    {118, 8.50f, 0.00f, 1.00f, 0.85f, false, "F7"},
    {119, 9.50f, 0.00f, 1.00f, 0.85f, false, "F8"},
    {120, 11.00f, 0.00f, 1.00f, 0.85f, false, "F9"},
    {121, 12.00f, 0.00f, 1.00f, 0.85f, false, "F10"},
    {122, 13.00f, 0.00f, 1.00f, 0.85f, false, "F11"},
    {123, 14.00f, 0.00f, 1.00f, 0.85f, false, "F12"},
    {44,  15.50f, 0.00f, 1.00f, 0.85f, false, "Prt"},
    {145, 16.50f, 0.00f, 1.00f, 0.85f, false, "Scr"},
    {19,  17.50f, 0.00f, 1.00f, 0.85f, false, "Pause"},

    // Row 1: Number Row (y = 1.15u, h = 1.00u)
    {192, 0.00f, 1.15f, 1.00f, 1.00f, true,  "`"},
    {49,  1.00f, 1.15f, 1.00f, 1.00f, true,  "1"},
    {50,  2.00f, 1.15f, 1.00f, 1.00f, true,  "2"},
    {51,  3.00f, 1.15f, 1.00f, 1.00f, true,  "3"},
    {52,  4.00f, 1.15f, 1.00f, 1.00f, true,  "4"},
    {53,  5.00f, 1.15f, 1.00f, 1.00f, true,  "5"},
    {54,  6.00f, 1.15f, 1.00f, 1.00f, true,  "6"},
    {55,  7.00f, 1.15f, 1.00f, 1.00f, true,  "7"},
    {56,  8.00f, 1.15f, 1.00f, 1.00f, true,  "8"},
    {57,  9.00f, 1.15f, 1.00f, 1.00f, true,  "9"},
    {48,  10.00f, 1.15f, 1.00f, 1.00f, true,  "0"},
    {189, 11.00f, 1.15f, 1.00f, 1.00f, true,  "-"},
    {187, 12.00f, 1.15f, 1.00f, 1.00f, true,  "="},
    {8,   13.00f, 1.15f, 2.00f, 1.00f, false, "Backspace"},
    {45,  15.50f, 1.15f, 1.00f, 1.00f, false, "Ins"},
    {36,  16.50f, 1.15f, 1.00f, 1.00f, false, "Home"},
    {33,  17.50f, 1.15f, 1.00f, 1.00f, false, "PgUp"},

    // Row 2: QWERTY Row (y = 2.21u, h = 1.00u)
    {9,   0.00f, 2.21f, 1.50f, 1.00f, false, "Tab"},
    {81,  1.50f, 2.21f, 1.00f, 1.00f, true,  "Q"},
    {87,  2.50f, 2.21f, 1.00f, 1.00f, true,  "W"},
    {69,  3.50f, 2.21f, 1.00f, 1.00f, true,  "E"},
    {82,  4.50f, 2.21f, 1.00f, 1.00f, true,  "R"},
    {84,  5.50f, 2.21f, 1.00f, 1.00f, true,  "T"},
    {89,  6.50f, 2.21f, 1.00f, 1.00f, true,  "Y"},
    {85,  7.50f, 2.21f, 1.00f, 1.00f, true,  "U"},
    {73,  8.50f, 2.21f, 1.00f, 1.00f, true,  "I"},
    {79,  9.50f, 2.21f, 1.00f, 1.00f, true,  "O"},
    {80,  10.50f, 2.21f, 1.00f, 1.00f, true,  "P"},
    {219, 11.50f, 2.21f, 1.00f, 1.00f, true,  "["},
    {221, 12.50f, 2.21f, 1.00f, 1.00f, true,  "]"},
    {220, 13.50f, 2.21f, 1.50f, 1.00f, true,  "\\"},
    {46,  15.50f, 2.21f, 1.00f, 1.00f, false, "Del"},
    {35,  16.50f, 2.21f, 1.00f, 1.00f, false, "End"},
    {34,  17.50f, 2.21f, 1.00f, 1.00f, false, "PgDn"},

    // Row 3: ASDF Home Row (y = 3.27u, h = 1.00u)
    {20,  0.00f, 3.27f, 1.75f, 1.00f, false, "Caps"},
    {65,  1.75f, 3.27f, 1.00f, 1.00f, true,  "A"},
    {83,  2.75f, 3.27f, 1.00f, 1.00f, true,  "S"},
    {68,  3.75f, 3.27f, 1.00f, 1.00f, true,  "D"},
    {70,  4.75f, 3.27f, 1.00f, 1.00f, true,  "F"},
    {71,  5.75f, 3.27f, 1.00f, 1.00f, true,  "G"},
    {72,  6.75f, 3.27f, 1.00f, 1.00f, true,  "H"},
    {74,  7.75f, 3.27f, 1.00f, 1.00f, true,  "J"},
    {75,  8.75f, 3.27f, 1.00f, 1.00f, true,  "K"},
    {76,  9.75f, 3.27f, 1.00f, 1.00f, true,  "L"},
    {186, 10.75f, 3.27f, 1.00f, 1.00f, true,  ";"},
    {222, 11.75f, 3.27f, 1.00f, 1.00f, true,  "'"},
    {13,  12.75f, 3.27f, 2.25f, 1.00f, false, "Enter"},

    // Row 4: ZXCV Row (y = 4.33u, h = 1.00u)
    {16,  0.00f, 4.33f, 2.25f, 1.00f, false, "Shift"},
    {90,  2.25f, 4.33f, 1.00f, 1.00f, true,  "Z"},
    {88,  3.25f, 4.33f, 1.00f, 1.00f, true,  "X"},
    {67,  4.25f, 4.33f, 1.00f, 1.00f, true,  "C"},
    {86,  5.25f, 4.33f, 1.00f, 1.00f, true,  "V"},
    {66,  6.25f, 4.33f, 1.00f, 1.00f, true,  "B"},
    {78,  7.25f, 4.33f, 1.00f, 1.00f, true,  "N"},
    {77,  8.25f, 4.33f, 1.00f, 1.00f, true,  "M"},
    {188, 9.25f, 4.33f, 1.00f, 1.00f, true,  ","},
    {190, 10.25f, 4.33f, 1.00f, 1.00f, true,  "."},
    {191, 11.25f, 4.33f, 1.00f, 1.00f, true,  "/"},
    {16,  12.25f, 4.33f, 2.75f, 1.00f, false, "Shift"},
    {38,  16.50f, 4.33f, 1.00f, 1.00f, false, "Up"},

    // Row 5: Space Row (y = 5.39u, h = 1.00u)
    {17,  0.00f, 5.39f, 1.25f, 1.00f, false, "Ctrl"},
    {91,  1.25f, 5.39f, 1.25f, 1.00f, false, "Win"},
    {18,  2.50f, 5.39f, 1.25f, 1.00f, false, "Alt"},
    {32,  3.75f, 5.39f, 6.25f, 1.00f, true,  "Space"},
    {18,  10.00f, 5.39f, 1.25f, 1.00f, false, "Alt"},
    {92,  11.25f, 5.39f, 1.25f, 1.00f, false, "Win"},
    {93,  12.50f, 5.39f, 1.25f, 1.00f, false, "Menu"},
    {17,  13.75f, 5.39f, 1.25f, 1.00f, false, "Ctrl"},
    {37,  15.50f, 5.39f, 1.00f, 1.00f, false, "Left"},
    {40,  16.50f, 5.39f, 1.00f, 1.00f, false, "Down"},
    {39,  17.50f, 5.39f, 1.00f, 1.00f, false, "Right"}
};

constexpr float KB_TOTAL_WIDTH_UNITS  = 18.50f;
constexpr float KB_TOTAL_HEIGHT_UNITS = 6.39f;

bool KeyboardView::Render(const LayoutData& layout, ImVec2 size, int& clickedKeyCode) {
    bool clicked = false;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);

    ImGui::InvisibleButton("##KeyboardCanvas", size);
    bool canvas_hovered = ImGui::IsItemHovered();
    ImVec2 mouse_pos = ImGui::GetIO().MousePos;

    const bool dark = g_DarkMode;

    // Palette: Clean Light Studio Keyboard (Default) vs High-Contrast Dark
    const ImU32 col_canvas_bg      = dark ? IM_COL32(18, 22, 30, 255)   : IM_COL32(244, 247, 252, 255);
    const ImU32 col_chassis_shadow = dark ? IM_COL32(10, 12, 16, 160)  : IM_COL32(185, 198, 215, 120);
    const ImU32 col_chassis_bg     = dark ? IM_COL32(28, 34, 46, 255)   : IM_COL32(228, 234, 244, 255);
    const ImU32 col_chassis_border = dark ? IM_COL32(48, 58, 78, 255)   : IM_COL32(202, 214, 228, 255);
    const ImU32 col_plate_bg       = dark ? IM_COL32(20, 24, 34, 255)   : IM_COL32(216, 224, 236, 255);

    // Keycaps: Crisp pure white in light mode with 3D tactile bevel
    const ImU32 col_key_type_base  = dark ? IM_COL32(24, 29, 40, 255)   : IM_COL32(195, 206, 222, 255);
    const ImU32 col_key_type_face  = dark ? IM_COL32(44, 53, 72, 255)   : IM_COL32(255, 255, 255, 255);
    const ImU32 col_key_type_rim   = dark ? IM_COL32(66, 78, 104, 180)  : IM_COL32(255, 255, 255, 255);
    const ImU32 col_key_type_bord  = dark ? IM_COL32(56, 68, 90, 255)   : IM_COL32(214, 222, 234, 255);

    // Modifier keys: Soft slate-gray keycaps
    const ImU32 col_key_mod_base   = dark ? IM_COL32(18, 22, 32, 255)   : IM_COL32(185, 196, 212, 255);
    const ImU32 col_key_mod_face   = dark ? IM_COL32(32, 38, 52, 255)   : IM_COL32(236, 241, 248, 255);
    const ImU32 col_key_mod_rim    = dark ? IM_COL32(50, 60, 80, 180)   : IM_COL32(250, 252, 255, 255);
    const ImU32 col_key_mod_bord   = dark ? IM_COL32(44, 54, 74, 255)   : IM_COL32(204, 214, 228, 255);

    // Hover & Active States
    const ImU32 col_hover_face     = dark ? IM_COL32(56, 68, 92, 255)   : IM_COL32(238, 246, 255, 255);
    const ImU32 col_hover_border   = dark ? IM_COL32(56, 189, 248, 255) : IM_COL32(37, 99, 235, 255);
    const ImU32 col_active_face    = dark ? IM_COL32(30, 36, 50, 255)   : IM_COL32(222, 234, 250, 255);

    // High Contrast Text Colors
    const ImU32 col_txt_normal     = dark ? IM_COL32(255, 255, 255, 255) : IM_COL32(15, 23, 42, 255);   // Deep slate #0F172A
    const ImU32 col_txt_shift      = dark ? IM_COL32(56, 189, 248, 255)  : IM_COL32(29, 78, 216, 255);  // Royal blue #1D4ED8
    const ImU32 col_txt_base_ref   = dark ? IM_COL32(148, 163, 184, 255) : IM_COL32(100, 116, 139, 255);// Slate gray #64748B
    const ImU32 col_txt_modifier   = dark ? IM_COL32(203, 213, 225, 255) : IM_COL32(51, 65, 85, 255);   // Dark slate #334155

    // Draw Canvas Base
    draw_list->AddRectFilled(p0, p1, col_canvas_bg, 8.0f);
    draw_list->AddRect(p0, p1, col_chassis_border, 8.0f, 0, 1.0f);

    // Scale keyboard to fill the canvas comfortably
    const float chassis_pad_x = 12.0f;
    const float chassis_pad_y = 10.0f;

    const float avail_w = size.x - 2.0f * chassis_pad_x;
    const float avail_h = size.y - 2.0f * chassis_pad_y;

    if (avail_w <= 100.0f || avail_h <= 100.0f) {
        return false;
    }

    // Dynamic scaling that fills the width/height nicely without fixed clamping
    float u = std::min(avail_w / KB_TOTAL_WIDTH_UNITS, avail_h / KB_TOTAL_HEIGHT_UNITS);
    u = std::clamp(u, 26.0f, 100.0f);

    const float kb_w = KB_TOTAL_WIDTH_UNITS * u;
    const float kb_h = KB_TOTAL_HEIGHT_UNITS * u;

    // Centered chassis position
    const float chassis_x0 = p0.x + (size.x - kb_w) * 0.5f - 12.0f;
    const float chassis_y0 = p0.y + (size.y - kb_h) * 0.5f - 10.0f;
    const float chassis_x1 = chassis_x0 + kb_w + 24.0f;
    const float chassis_y1 = chassis_y0 + kb_h + 20.0f;

    // 1. Chassis Elevation Drop Shadow
    draw_list->AddRectFilled(ImVec2(chassis_x0 - 2, chassis_y0 + 3), ImVec2(chassis_x1 + 2, chassis_y1 + 7), col_chassis_shadow, 12.0f);

    // 2. Chassis Base & Bezel
    draw_list->AddRectFilled(ImVec2(chassis_x0, chassis_y0), ImVec2(chassis_x1, chassis_y1), col_chassis_bg, 10.0f);
    draw_list->AddRect(ImVec2(chassis_x0, chassis_y0), ImVec2(chassis_x1, chassis_y1), col_chassis_border, 10.0f, 0, 1.5f);

    // 3. Recessed Switch Plate
    draw_list->AddRectFilled(ImVec2(chassis_x0 + 5, chassis_y0 + 5), ImVec2(chassis_x1 - 5, chassis_y1 - 5), col_plate_bg, 7.0f);

    const float key_origin_x = chassis_x0 + 12.0f;
    const float key_origin_y = chassis_y0 + 10.0f;

    const float key_margin = std::max(2.0f, u * 0.045f);
    const float bevel_depth = std::max(2.5f, u * 0.075f);

    for (const auto& k : g_BaseKeys) {
        const float kx = key_origin_x + k.ux * u;
        const float ky = key_origin_y + k.uy * u;
        const float kw = k.uw * u;
        const float kh = k.uh * u;

        const ImVec2 kp0 = ImVec2(kx + key_margin, ky + key_margin);
        const ImVec2 kp1 = ImVec2(kx + kw - key_margin, ky + kh - key_margin);

        const bool hovered = canvas_hovered &&
                             mouse_pos.x >= kp0.x && mouse_pos.x <= kp1.x &&
                             mouse_pos.y >= kp0.y && mouse_pos.y <= kp1.y;

        const bool mouse_down = hovered && ImGui::IsMouseDown(0);

        if (hovered && k.modifiable) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }

        if (hovered && ImGui::IsMouseClicked(0) && k.modifiable) {
            clickedKeyCode = k.keyCode;
            clicked = true;
        }

        // Color selection
        ImU32 col_base   = k.modifiable ? col_key_type_base : col_key_mod_base;
        ImU32 col_face   = k.modifiable ? col_key_type_face : col_key_mod_face;
        ImU32 col_rim    = k.modifiable ? col_key_type_rim  : col_key_mod_rim;
        ImU32 col_border = k.modifiable ? col_key_type_bord : col_key_mod_bord;

        if (hovered && k.modifiable) {
            col_face = col_hover_face;
            col_border = col_hover_border;
        }

        if (mouse_down && k.modifiable) {
            col_face = col_active_face;
        }

        // 1. Keycap 3D Bevel Base (bottom layer)
        draw_list->AddRectFilled(kp0, kp1, col_base, 5.0f);

        // 2. Keycap Top Raised Face (shifted up by bevel depth unless pressed)
        ImVec2 face_p0 = kp0;
        ImVec2 face_p1 = ImVec2(kp1.x, kp1.y - (mouse_down ? 1.0f : bevel_depth));
        if (mouse_down) {
            face_p0.y += 1.5f;
            face_p1.y += 1.5f;
        }

        draw_list->AddRectFilled(face_p0, face_p1, col_face, 4.5f);

        // 3. Subtle ambient light highlight rim along top edge
        draw_list->AddLine(
            ImVec2(face_p0.x + 3.0f, face_p0.y + 0.5f),
            ImVec2(face_p1.x - 3.0f, face_p0.y + 0.5f),
            col_rim, 1.0f
        );

        // 4. Keycap Outline
        draw_list->AddRect(face_p0, face_p1, col_border, 4.5f, 0, 1.0f);

        // Render Keycap Text
        if (k.modifiable) {
            auto it = layout.key.find(k.keyCode);
            std::string normal = (it != layout.key.end()) ? WStringToUtf8(it->second.normal) : "";
            std::string shift  = (it != layout.key.end()) ? WStringToUtf8(it->second.shift)  : "";

            if (k.keyCode == 32) {
                // Spacebar: centered clean label
                if (g_FontKeySub) ImGui::PushFont(g_FontKeySub);
                const char* spaceText = (!normal.empty() && normal != " ") ? normal.c_str() : "Space";
                ImVec2 spaceSize = ImGui::CalcTextSize(spaceText);
                ImVec2 spacePos = ImVec2(
                    face_p0.x + (face_p1.x - face_p0.x - spaceSize.x) * 0.5f,
                    face_p0.y + (face_p1.y - face_p0.y - spaceSize.y) * 0.5f
                );
                draw_list->AddText(spacePos, col_txt_modifier, spaceText);
                if (g_FontKeySub) ImGui::PopFont();
            } else {
                // Top-Right: Physical US key reference (e.g. "Q", "1")
                if (g_FontKeySmall) ImGui::PushFont(g_FontKeySmall);
                ImVec2 baseSize = ImGui::CalcTextSize(k.label);
                ImVec2 basePos = ImVec2(face_p1.x - baseSize.x - 4.5f, face_p0.y + 3.5f);
                draw_list->AddText(basePos, col_txt_base_ref, k.label);
                if (g_FontKeySmall) ImGui::PopFont();

                // Top-Left: Shifted character (e.g. "ং", "!")
                if (!shift.empty()) {
                    if (g_FontKeySub) ImGui::PushFont(g_FontKeySub);
                    ImVec2 shiftPos = ImVec2(face_p0.x + 5.5f, face_p0.y + 3.5f);
                    draw_list->AddText(shiftPos, col_txt_shift, shift.c_str());
                    if (g_FontKeySub) ImGui::PopFont();
                }

                // Bottom-Right: Normal / primary character (e.g. "ঙ", "১")
                if (!normal.empty()) {
                    if (g_FontKeyLarge) ImGui::PushFont(g_FontKeyLarge);
                    ImVec2 normSize = ImGui::CalcTextSize(normal.c_str());
                    ImVec2 normPos = ImVec2(face_p1.x - normSize.x - 5.5f, face_p1.y - normSize.y - 4.0f);
                    draw_list->AddText(normPos, col_txt_normal, normal.c_str());
                    if (g_FontKeyLarge) ImGui::PopFont();
                }

                // If key has no custom normal or shift character, center the base key
                if (normal.empty() && shift.empty()) {
                    if (g_FontKeySub) ImGui::PushFont(g_FontKeySub);
                    ImVec2 centerSize = ImGui::CalcTextSize(k.label);
                    ImVec2 centerPos = ImVec2(
                        face_p0.x + (face_p1.x - face_p0.x - centerSize.x) * 0.5f,
                        face_p0.y + (face_p1.y - face_p0.y - centerSize.y) * 0.5f
                    );
                    draw_list->AddText(centerPos, col_txt_base_ref, k.label);
                    if (g_FontKeySub) ImGui::PopFont();
                }
            }

            // Interactive Tooltip on hover
            if (hovered) {
                ImGui::BeginTooltip();
                ImGui::Text("Key: [%s] (Code: %d)", k.label, k.keyCode);
                ImGui::Separator();
                if (!normal.empty()) ImGui::Text("Normal : %s", normal.c_str());
                if (!shift.empty())  ImGui::Text("Shift  : %s", shift.c_str());
                ImGui::TextColored(ImVec4(0.1f, 0.6f, 0.3f, 1.0f), "Click to edit mapping");
                ImGui::EndTooltip();
            }
        } else {
            // Modifier / Functional Key: Clean centered label
            if (g_FontKeySub) ImGui::PushFont(g_FontKeySub);
            ImVec2 modSize = ImGui::CalcTextSize(k.label);
            ImVec2 modPos = ImVec2(
                face_p0.x + (face_p1.x - face_p0.x - modSize.x) * 0.5f,
                face_p0.y + (face_p1.y - face_p0.y - modSize.y) * 0.5f
            );
            draw_list->AddText(modPos, col_txt_modifier, k.label);
            if (g_FontKeySub) ImGui::PopFont();
        }
    }

    return clicked;
}

} // namespace editor

