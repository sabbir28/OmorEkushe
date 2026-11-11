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
std::string MainWindow::s_statusMessage = "Ready. Click any key to edit character mapping.";

void MainWindow::Initialize() {
    RefreshLayouts();
}

void MainWindow::RefreshLayouts() {
    s_layouts = editor::FindLayouts();
    if (!s_layouts.empty()) {
        s_currentLayoutIndex = 0;
        // Prefer selecting Unicode layout by default for native Bengali visualization
        for (int i = 0; i < static_cast<int>(s_layouts.size()); ++i) {
            if (s_layouts[i].name.find(L"Unicode") != std::wstring::npos) {
                s_currentLayoutIndex = i;
                break;
            }
        }
        UpdateLayoutView();
    }
}

void MainWindow::UpdateLayoutView() {
    if (s_currentLayoutIndex >= 0 && s_currentLayoutIndex < static_cast<int>(s_layouts.size())) {
        std::string name = editor::WStringToUtf8(s_layouts[s_currentLayoutIndex].name);
        strncpy_s(s_editName, sizeof(s_editName), name.c_str(), _TRUNCATE);
        std::string icon = editor::WStringToUtf8(s_layouts[s_currentLayoutIndex].iconName);
        strncpy_s(s_editIcon, sizeof(s_editIcon), icon.c_str(), _TRUNCATE);
    }
}

bool MainWindow::Render() {
    bool shouldExit = false;
    ImGuiIO& io = ImGui::GetIO();

    // Theme mode synchronization
    if (editor::g_DarkMode) {
        ImGui::StyleColorsDark();
    } else {
        ImGui::StyleColorsLight();
    }

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | 
                                    ImGuiWindowFlags_NoMove | 
                                    ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoSavedSettings;

    const ImVec4 winBg = editor::g_DarkMode 
        ? ImVec4(0.08f, 0.10f, 0.14f, 1.0f) 
        : ImVec4(0.95f, 0.96f, 0.98f, 1.0f);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, winBg);

    if (ImGui::Begin("###MainView", nullptr, window_flags)) {
        // =========================================================
        // Header Panel (App branding, layout switcher & metadata)
        // =========================================================
        const ImVec4 cardBg = editor::g_DarkMode 
            ? ImVec4(0.12f, 0.15f, 0.20f, 1.0f) 
            : ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
        const ImVec4 cardBorder = editor::g_DarkMode 
            ? ImVec4(0.18f, 0.22f, 0.30f, 1.0f) 
            : ImVec4(0.85f, 0.88f, 0.92f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBg);
        ImGui::PushStyleColor(ImGuiCol_Border, cardBorder);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14, 10));

        if (ImGui::BeginChild("HeaderPanel", ImVec2(0, 86), true)) {
            // Row 1: App Title & Badges
            ImGui::TextColored(editor::g_DarkMode ? ImVec4(0.95f, 0.97f, 1.0f, 1.0f) : ImVec4(0.08f, 0.12f, 0.20f, 1.0f),
                               "Omor Ekushe  |  Keyboard Layout Editor");
            
            // Layout Type Badge
            if (s_currentLayoutIndex >= 0 && s_currentLayoutIndex < static_cast<int>(s_layouts.size())) {
                const auto& cur = s_layouts[s_currentLayoutIndex];
                ImGui::SameLine();
                if (cur.name.find(L"Unicode") != std::wstring::npos) {
                    ImGui::TextColored(ImVec4(0.05f, 0.65f, 0.35f, 1.0f), "[ Unicode Layout ]");
                } else if (cur.name.find(L"Classic") != std::wstring::npos) {
                    ImGui::TextColored(ImVec4(0.15f, 0.45f, 0.90f, 1.0f), "[ Classic / ANSI ]");
                } else {
                    ImGui::TextColored(ImVec4(0.85f, 0.55f, 0.10f, 1.0f), "[ Custom Layout ]");
                }

                ImGui::SameLine();
                ImGui::TextDisabled("• %zu Mapped Keys", cur.key.size());
            }

            // Theme Toggle Button on Top-Right
            const char* themeLabel = editor::g_DarkMode ? "Light Mode" : "Dark Mode";
            float themeBtnWidth = 110.0f;
            ImGui::SameLine(ImGui::GetWindowWidth() - themeBtnWidth - 14.0f);
            if (ImGui::Button(themeLabel, ImVec2(themeBtnWidth, 24))) {
                editor::g_DarkMode = !editor::g_DarkMode;
            }

            ImGui::Spacing();

            // Row 2: Layout Controls (Base Layout combo, Name input, Icon input)
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Base Layout:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(220.0f);

            // Store preview in local std::string to ensure pointer lifetime throughout BeginCombo
            std::string currentLayoutName = "Select Layout";
            if (s_currentLayoutIndex >= 0 && s_currentLayoutIndex < static_cast<int>(s_layouts.size())) {
                currentLayoutName = editor::WStringToUtf8(s_layouts[s_currentLayoutIndex].name);
            }

            if (ImGui::BeginCombo("##BaseLayout", currentLayoutName.c_str())) {
                for (int i = 0; i < static_cast<int>(s_layouts.size()); ++i) {
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

            ImGui::SameLine(0, 18);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Name:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(170.0f);
            if (ImGui::InputText("##LayoutName", s_editName, sizeof(s_editName))) {
                if (s_currentLayoutIndex >= 0) {
                    s_layouts[s_currentLayoutIndex].name = editor::Utf8ToWString(s_editName);
                }
            }

            ImGui::SameLine(0, 18);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Icon:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(110.0f);
            if (ImGui::InputText("##IconName", s_editIcon, sizeof(s_editIcon))) {
                if (s_currentLayoutIndex >= 0) {
                    s_layouts[s_currentLayoutIndex].iconName = editor::Utf8ToWString(s_editIcon);
                }
            }

            ImGui::SameLine(0, 14);
            if (ImGui::Button("Reload", ImVec2(72, 0))) {
                RefreshLayouts();
                s_statusMessage = "Reloaded layout files.";
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);

        ImGui::Spacing();

        // =========================================================
        // Keyboard Layout Preview Header
        // =========================================================
        ImGui::TextColored(editor::g_DarkMode ? ImVec4(0.85f, 0.90f, 0.95f, 1.0f) : ImVec4(0.15f, 0.20f, 0.30f, 1.0f),
                           "Keyboard Layout Preview");
        ImGui::SameLine();
        ImGui::TextDisabled("— Click any keycap to customize its normal and shifted characters");

        // Available canvas size (leaving room for bottom action bar)
        ImVec2 size = ImGui::GetContentRegionAvail();
        size.y -= 46.0f; // Reserved for bottom action toolbar

        if (s_currentLayoutIndex >= 0 && s_currentLayoutIndex < static_cast<int>(s_layouts.size())) {
            if (editor::KeyboardView::Render(s_layouts[s_currentLayoutIndex], size, s_selectedKeyCode)) {
                s_showKeyEditPopup = true;
            }
        }

        ImGui::Spacing();

        // =========================================================
        // Bottom Action Toolbar
        // =========================================================
        if (ImGui::Button("New Layout", ImVec2(130, 34))) {
            editor::LayoutData newLayout;
            newLayout.name = L"New Layout";
            s_layouts.push_back(newLayout);
            s_currentLayoutIndex = static_cast<int>(s_layouts.size()) - 1;
            UpdateLayoutView();
            s_statusMessage = "Created new custom layout. Click keycaps to configure.";
        }

        ImGui::SameLine(0, 10);
        // Primary Save Button highlighted in modern emerald accent
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.55f, 0.38f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.12f, 0.68f, 0.46f, 1.0f));
        if (ImGui::Button("Save Layout", ImVec2(140, 34))) {
            if (s_currentLayoutIndex >= 0) {
                auto& layout = s_layouts[s_currentLayoutIndex];
                if (layout.path.empty()) {
                    std::wstring dir = editor::GetExeDirectory() + L"Layouts\\";
                    CreateDirectoryW(dir.c_str(), nullptr);
                    layout.path = dir + layout.name + L".xml";
                }
                if (layout.saveToFile(layout.path.c_str())) {
                    s_statusMessage = "Layout saved successfully to: " + editor::WStringToUtf8(layout.path);
                } else {
                    s_statusMessage = "Failed to write layout file.";
                }
            }
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine(0, 20);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", s_statusMessage.c_str());

        // Exit button aligned on the far right
        ImGui::SameLine(ImGui::GetWindowWidth() - 130.0f);
        if (ImGui::Button("Exit", ImVec2(116, 34))) {
            shouldExit = true;
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(); // WindowBg

    // Modal Dialogs
    if (s_showKeyEditPopup && s_currentLayoutIndex >= 0) {
        if (editor::KeyEditDialog::Render(s_layouts[s_currentLayoutIndex], s_selectedKeyCode, s_showKeyEditPopup)) {
            s_statusMessage = "Key mapping updated.";
        }
    }

    return shouldExit;
}

void MainWindow::Shutdown() {
}

