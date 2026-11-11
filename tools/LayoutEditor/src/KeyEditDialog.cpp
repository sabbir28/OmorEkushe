#include "KeyEditDialog.h"
#include "imgui.h"
#include "Utf8Utils.h"
#include "KeyboardView.h"

namespace editor {

bool KeyEditDialog::Render(LayoutData& layout, int keyCode, bool& show) {
    bool updated = false;
    
    if (show) {
        ImGui::OpenPopup("Edit Key Mapping");
    }

    ImGui::SetNextWindowSize(ImVec2(380, 0), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Edit Key Mapping", &show, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto& mapping = layout.key[keyCode];

        static char normalBuf[256] = "";
        static char shiftBuf[256] = "";
        static char normalOptBuf[256] = "";
        static char shiftOptBuf[256] = "";
        static int lastKeyCode = -1;

        if (lastKeyCode != keyCode) {
            strncpy_s(normalBuf, sizeof(normalBuf), WStringToUtf8(mapping.normal).c_str(), _TRUNCATE);
            strncpy_s(shiftBuf, sizeof(shiftBuf), WStringToUtf8(mapping.shift).c_str(), _TRUNCATE);
            strncpy_s(normalOptBuf, sizeof(normalOptBuf), WStringToUtf8(mapping.normalOption).c_str(), _TRUNCATE);
            strncpy_s(shiftOptBuf, sizeof(shiftOptBuf), WStringToUtf8(mapping.shiftOption).c_str(), _TRUNCATE);
            lastKeyCode = keyCode;
        }

        // Header info
        ImGui::TextColored(ImVec4(0.22f, 0.74f, 0.97f, 1.0f), "Virtual KeyCode: %d", keyCode);
        ImGui::SameLine();
        ImGui::TextDisabled("(Hex: 0x%02X)", keyCode);

        ImGui::Separator();
        ImGui::Spacing();

        // Live Keycap Preview Box
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 previewPos = ImGui::GetCursorScreenPos();
        previewPos.x += (ImGui::GetContentRegionAvail().x - 64.0f) * 0.5f;
        ImVec2 previewEnd = ImVec2(previewPos.x + 64.0f, previewPos.y + 60.0f);

        // Draw preview keycap
        draw_list->AddRectFilled(previewPos, previewEnd, IM_COL32(20, 24, 33, 255), 6.0f);
        draw_list->AddRectFilled(previewPos, ImVec2(previewEnd.x, previewEnd.y - 4.0f), IM_COL32(36, 43, 58, 255), 5.5f);
        draw_list->AddRect(previewPos, ImVec2(previewEnd.x, previewEnd.y - 4.0f), IM_COL32(56, 189, 248, 255), 5.5f, 0, 1.0f);

        // Draw Shift preview char (Top-Left)
        if (shiftBuf[0] != '\0') {
            if (g_FontKeySub) ImGui::PushFont(g_FontKeySub);
            draw_list->AddText(ImVec2(previewPos.x + 6.0f, previewPos.y + 4.0f), IM_COL32(125, 211, 252, 255), shiftBuf);
            if (g_FontKeySub) ImGui::PopFont();
        }

        // Draw Normal preview char (Bottom-Right)
        if (normalBuf[0] != '\0') {
            if (g_FontKeyLarge) ImGui::PushFont(g_FontKeyLarge);
            ImVec2 nSize = ImGui::CalcTextSize(normalBuf);
            draw_list->AddText(ImVec2(previewEnd.x - nSize.x - 6.0f, previewEnd.y - nSize.y - 7.0f), IM_COL32(248, 250, 252, 255), normalBuf);
            if (g_FontKeyLarge) ImGui::PopFont();
        }

        ImGui::Dummy(ImVec2(0, 68.0f));

        // Input Fields
        ImGui::Text("Normal Character (Unshifted):");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##Normal", normalBuf, sizeof(normalBuf));

        ImGui::Spacing();
        ImGui::Text("Shift Character (With Shift):");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##Shift", shiftBuf, sizeof(shiftBuf));

        ImGui::Spacing();
        ImGui::Text("Normal Option (Advanced):");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##NormalOpt", normalOptBuf, sizeof(normalOptBuf));

        ImGui::Spacing();
        ImGui::Text("Shift Option (Advanced):");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##ShiftOpt", shiftOptBuf, sizeof(shiftOptBuf));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Action Buttons
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.52f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.65f, 0.44f, 1.0f));
        if (ImGui::Button("Apply Changes", ImVec2(150, 32))) {
            mapping.normal = Utf8ToWString(normalBuf);
            mapping.shift = Utf8ToWString(shiftBuf);
            mapping.normalOption = Utf8ToWString(normalOptBuf);
            mapping.shiftOption = Utf8ToWString(shiftOptBuf);
            show = false;
            updated = true;
            ImGui::CloseCurrentPopup();
            lastKeyCode = -1;
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 32))) {
            show = false;
            ImGui::CloseCurrentPopup();
            lastKeyCode = -1;
        }

        ImGui::EndPopup();
    }
    
    return updated;
}

} // namespace editor

