#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <atomic>
#include <shlobj.h>
#include "InstallerLogic.h"
#include <windows.h>
#include <string>
#include <vector>

enum class WizardPage {
    Welcome,
    Options,
    Progress,
    Finished
};

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(600, 450, "OmorEkushe Setup", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    WizardPage currentPage = WizardPage::Welcome;
    InstallerLogic::Options options;
    wchar_t programFiles[MAX_PATH];
    SHGetSpecialFolderPathW(NULL, programFiles, CSIDL_PROGRAM_FILES, FALSE);
    options.installPath = std::wstring(programFiles) + L"\\OmorEkushe";
    options.createDesktopShortcut = true;
    options.runOnStartup = true;

    bool should_close = false;
    char path_buf[512];
    WideCharToMultiByte(CP_UTF8, 0, options.installPath.c_str(), -1, path_buf, 512, NULL, NULL);

    std::vector<std::string> install_logs;
    std::string last_logged_file = "";

    glfwShowWindow(window);

    while (!glfwWindowShouldClose(window) && !should_close) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Setup", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        // Header Title across all menus
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "OmorEkushe Setup Wizard");
        ImGui::Separator();
        ImGui::Spacing();

        if (currentPage == WizardPage::Welcome) {
            ImGui::Text("Welcome to the OmorEkushe Setup Wizard");
            ImGui::Spacing();
            ImGui::TextWrapped("This wizard will install OmorEkushe on your computer.\n\n"
                               "OmorEkushe is a specialized interactive application providing next-generation smart Bengali keyboard layout configurations, bringing a clean UI and efficient typing engine to modern Windows platforms.");
            ImGui::Spacing();
            ImGui::TextWrapped("Click Next to continue with the installation.");
            
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 50.0f);
            ImGui::Separator();
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 130.0f, ImGui::GetWindowHeight() - 40.0f));
            if (ImGui::Button("Next >", ImVec2(110, 30))) {
                currentPage = WizardPage::Options;
            }
        } 
        else if (currentPage == WizardPage::Options) {
            ImGui::Text("Installation Options");
            ImGui::Spacing();
            ImGui::TextWrapped("Select the destination folder where OmorEkushe will be installed, and choose any additional options.");
            ImGui::Spacing();

            ImGui::Text("Select Installation Folder:");
            ImGui::InputText("##path", path_buf, 512);
            
            ImGui::Spacing();
            ImGui::Checkbox("Create Desktop Shortcut", &options.createDesktopShortcut);
            ImGui::Checkbox("Start with Windows (Auto-boot)", &options.runOnStartup);

            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 50.0f);
            ImGui::Separator();
            
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 250.0f, ImGui::GetWindowHeight() - 40.0f));
            if (ImGui::Button("< Back", ImVec2(110, 30))) {
                currentPage = WizardPage::Welcome;
            }
            ImGui::SameLine();
            if (ImGui::Button("Install", ImVec2(110, 30))) {
                int len = MultiByteToWideChar(CP_UTF8, 0, path_buf, -1, NULL, 0);
                std::wstring wpath(len, 0);
                MultiByteToWideChar(CP_UTF8, 0, path_buf, -1, &wpath[0], len);
                options.installPath = wpath;
                
                install_logs.push_back("Starting installation...");
                install_logs.push_back("Target Directory: " + std::string(path_buf));
                
                InstallerLogic::StartInstallation(options, InstallerLogic::g_current_progress);
                currentPage = WizardPage::Progress;
            }
        } 
        else if (currentPage == WizardPage::Progress) {
            ImGui::Text("Installing OmorEkushe...");
            ImGui::Spacing();
            ImGui::ProgressBar(InstallerLogic::g_current_progress.percentage, ImVec2(-1.0f, 0.0f));
            ImGui::Spacing();

            // Track extract logs
            std::string file_utf8;
            int len = WideCharToMultiByte(CP_UTF8, 0, InstallerLogic::g_current_progress.currentFile.c_str(), -1, NULL, 0, NULL, NULL);
            file_utf8.resize(len);
            WideCharToMultiByte(CP_UTF8, 0, InstallerLogic::g_current_progress.currentFile.c_str(), -1, &file_utf8[0], len, NULL, NULL);
            
            if (file_utf8 != last_logged_file && !file_utf8.empty()) {
                install_logs.push_back("Extracting: " + file_utf8);
                last_logged_file = file_utf8;
            }

            // Draw scrolling console window
            ImGui::Text("Installation Log:");
            ImGui::BeginChild("LogRegion", ImVec2(0, -60), true, ImGuiWindowFlags_HorizontalScrollbar);
            for (const auto& log : install_logs) {
                ImGui::TextUnformatted(log.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();

            if (InstallerLogic::g_current_progress.finished) {
                install_logs.push_back("Finished extraction procedures.");
                currentPage = WizardPage::Finished;
            }
        }
        else if (currentPage == WizardPage::Finished) {
            if (InstallerLogic::g_current_progress.errorMessage.empty()) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Installation Successful!");
                ImGui::Spacing();
                ImGui::TextWrapped("OmorEkushe has successfully been installed on your system into:");
                ImGui::TextWrapped("%s", path_buf);
                ImGui::Spacing();
                ImGui::TextWrapped("You may now click Finish to close the setup wizard.");
                
                ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 50.0f);
                ImGui::Separator();
                ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 130.0f, ImGui::GetWindowHeight() - 40.0f));
                if (ImGui::Button("Finish", ImVec2(110, 30))) {
                    should_close = true;
                }
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Installation Failed:");
                ImGui::Spacing();
                ImGui::TextWrapped("Error details:");
                ImGui::TextWrapped("%s", InstallerLogic::g_current_progress.errorMessage.c_str());
                
                ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 50.0f);
                ImGui::Separator();
                ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 130.0f, ImGui::GetWindowHeight() - 40.0f));
                if (ImGui::Button("Close", ImVec2(110, 30))) {
                    should_close = true;
                }
            }
        }

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
