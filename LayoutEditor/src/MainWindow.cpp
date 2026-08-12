#include "MainWindow.h"
#include "imgui.h"
#include "KeyboardView.h"
#include "KeyEditDialog.h"
#include "Utf8Utils.h"
#include <shlwapi.h>
#include <filesystem>

std::vector<editor::LayoutData> MainWindow::s_layouts;
int MainWindow::s_currentLayoutIndex = -1;
char MainWindow::s_editName[256] = "";
char MainWindow::s_editIcon[256] = "";
bool MainWindow::s_showKeyEditPopup = false;
int MainWindow::s_selectedKeyCode = -1;

void MainWindow::Initialize() {
    RefreshLayouts();
}

void MainWindow::RefreshLayouts() {
    s_layouts = editor::FindLayouts();
    if (!s_layouts.empty()) {
        s_currentLayoutIndex = 0;
        UpdateLayoutView();
    }
}

void MainWindow::UpdateLayoutView() {
    if (s_currentLayoutIndex >= 0 && s_currentLayoutIndex < (int)s_layouts.size()) {
        std::string name = editor::WStringToUtf8(s_layouts[s_currentLayoutIndex].name);
        strncpy(s_editName, name.c_str(), sizeof(s_editName));
        std::string icon = editor::WStringToUtf8(s_layouts[s_currentLayoutIndex].iconName);
        strncpy(s_editIcon, icon.c_str(), sizeof(s_editIcon));
    }
}

bool MainWindow::Render() {
    bool shouldExit = false;
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(io.DisplaySize);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    
    if (ImGui::Begin("###MainView", nullptr, window_flags)) {
        // Layout Name Row
        ImGui::Text("Layout Name:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-80.0f);
        if (ImGui::InputText("##LayoutName", s_editName, sizeof(s_editName))) {
            if (s_currentLayoutIndex >= 0) {
                s_layouts[s_currentLayoutIndex].name = editor::Utf8ToWString(s_editName);
            }
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100.0f);
        if (ImGui::InputText("##IconName", s_editIcon, sizeof(s_editIcon))) {
            if (s_currentLayoutIndex >= 0) {
                s_layouts[s_currentLayoutIndex].iconName = editor::Utf8ToWString(s_editIcon);
            }
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Icon Name");
        

        // Base Layout Selection
        ImGui::Text("Base Layout:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        
        const char* preview = (s_currentLayoutIndex >= 0) ? editor::WStringToUtf8(s_layouts[s_currentLayoutIndex].name).c_str() : "Select Layout";
        if (ImGui::BeginCombo("##BaseLayout", preview)) {
            for (int i = 0; i < s_layouts.size(); i++) {
                bool is_selected = (s_currentLayoutIndex == i);
                std::string name = editor::WStringToUtf8(s_layouts[i].name);
                if (ImGui::Selectable(name.c_str(), is_selected)) {
                    s_currentLayoutIndex = i;
                    UpdateLayoutView();
                }
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        // Keyboard Preview Area
        ImGui::Text("Keyboard Layout Preview");
        ImVec2 size = ImGui::GetContentRegionAvail();
        size.y -= 40; // Space for bottom buttons
        
        if (s_currentLayoutIndex >= 0) {
            if (editor::KeyboardView::Render(s_layouts[s_currentLayoutIndex], size, s_selectedKeyCode)) {
                s_showKeyEditPopup = true;
            }
        }

        // Bottom Buttons
        if (ImGui::Button("New Layout", ImVec2(140, 35))) {
            editor::LayoutData newLayout;
            newLayout.name = L"New Layout";
            s_layouts.push_back(newLayout);
            s_currentLayoutIndex = (int)s_layouts.size() - 1;
            UpdateLayoutView();
        }
        ImGui::SameLine();
        if (ImGui::Button("Save Layout", ImVec2(140, 35))) {
            if (s_currentLayoutIndex >= 0) {
                auto& layout = s_layouts[s_currentLayoutIndex];
                if (layout.path.empty()) {
                    std::wstring dir = editor::GetExeDirectory() + L"Layouts\\";
                    CreateDirectoryW(dir.c_str(), nullptr);
                    layout.path = dir + layout.name + L".xml";
                }
                layout.saveToFile(layout.path.c_str());
            }
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 150);
        if (ImGui::Button("Exit", ImVec2(140, 35))) {
            shouldExit = true;
        }
    }
    ImGui::End();

    // Modals
    if (s_showKeyEditPopup) {
         if (editor::KeyEditDialog::Render(s_layouts[s_currentLayoutIndex], s_selectedKeyCode, s_showKeyEditPopup)) {
             // Dialog closed or updated
         }
    }
    return shouldExit;
}

void MainWindow::Shutdown() {
}
