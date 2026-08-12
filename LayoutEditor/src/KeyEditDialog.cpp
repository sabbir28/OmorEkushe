#include "KeyEditDialog.h"
#include "imgui.h"
#include "Utf8Utils.h"

namespace editor {

bool KeyEditDialog::Render(LayoutData& layout, int keyCode, bool& show) {
    bool updated = false;
    
    if (show) {
        ImGui::OpenPopup("Edit Key Mapping");
    }

    if (ImGui::BeginPopupModal("Edit Key Mapping", &show, ImGuiWindowFlags_AlwaysAutoResize)) {
        auto& mapping = layout.key[keyCode];

        static char normalBuf[256] = "";
        static char shiftBuf[256] = "";
        static char normalOptBuf[256] = "";
        static char shiftOptBuf[256] = "";
        static int lastKeyCode = -1;

        if (lastKeyCode != keyCode) {
            strncpy(normalBuf, WStringToUtf8(mapping.normal).c_str(), sizeof(normalBuf));
            strncpy(shiftBuf, WStringToUtf8(mapping.shift).c_str(), sizeof(shiftBuf));
            strncpy(normalOptBuf, WStringToUtf8(mapping.normalOption).c_str(), sizeof(normalOptBuf));
            strncpy(shiftOptBuf, WStringToUtf8(mapping.shiftOption).c_str(), sizeof(shiftOptBuf));
            lastKeyCode = keyCode;
        }

        ImGui::Text("Key Code: %d", keyCode);
        ImGui::Separator();

        ImGui::InputText("Normal", normalBuf, sizeof(normalBuf));
        ImGui::InputText("Shift", shiftBuf, sizeof(shiftBuf));
        ImGui::InputText("Normal Opt", normalOptBuf, sizeof(normalOptBuf));
        ImGui::InputText("Shift Opt", shiftOptBuf, sizeof(shiftOptBuf));

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            mapping.normal = Utf8ToWString(normalBuf);
            mapping.shift = Utf8ToWString(shiftBuf);
            mapping.normalOption = Utf8ToWString(normalOptBuf);
            mapping.shiftOption = Utf8ToWString(shiftOptBuf);
            show = false;
            updated = true;
            ImGui::CloseCurrentPopup();
            lastKeyCode = -1;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            show = false;
            ImGui::CloseCurrentPopup();
            lastKeyCode = -1;
        }

        ImGui::EndPopup();
    }
    
    return updated;
}

} // namespace editor
