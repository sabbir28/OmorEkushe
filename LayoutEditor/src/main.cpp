#include <iostream>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "MainWindow.h"
#include "LayoutEditor.h"
#include "KeyboardView.h"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**) {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context (980x680 for comfortable keyboard layout viewing)
    constexpr int INIT_WIN_W = 980;
    constexpr int INIT_WIN_H = 680;
    GLFWwindow* window = glfwCreateWindow(INIT_WIN_W, INIT_WIN_H, "Omor Ekushe - Keyboard Layout Editor", NULL, NULL);
    if (window == NULL)
        return 1;

    // Set minimum size limits so the visual keyboard layout canvas is not squashed
    glfwSetWindowSizeLimits(window, 860, 560, GLFW_DONT_CARE, GLFW_DONT_CARE);

    // Center the GLFW window on the primary monitor
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (primaryMonitor) {
        const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
        if (mode) {
            int monitorX = 0, monitorY = 0;
            glfwGetMonitorPos(primaryMonitor, &monitorX, &monitorY);
            glfwSetWindowPos(window,
                monitorX + (mode->width - INIT_WIN_W) / 2,
                monitorY + (mode->height - INIT_WIN_H) / 2);
        }
    }

#ifdef _WIN32
    // Enable modern dark title bar on Windows 10/11
    HWND hwnd = glfwGetWin32Window(window);
    if (hwnd) {
        BOOL useDarkMode = TRUE;
        if (FAILED(DwmSetWindowAttribute(hwnd, 20 /* DWMWA_USE_IMMERSIVE_DARK_MODE */, &useDarkMode, sizeof(useDarkMode)))) {
            DwmSetWindowAttribute(hwnd, 19 /* legacy flag */, &useDarkMode, sizeof(useDarkMode));
        }
    }
#endif

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Load Unicode & Indic fonts with comprehensive glyph ranges
    char win_dir[512] = "";
#ifdef _WIN32
    GetWindowsDirectoryA(win_dir, sizeof(win_dir));
#else
    strncpy_s(win_dir, sizeof(win_dir), "/usr/share", _TRUNCATE);
#endif

    char font_nirmala[512];
    char font_nirmala_b[512];
    char font_segoe[512];
    snprintf(font_nirmala,   sizeof(font_nirmala),   "%s\\Fonts\\Nirmala.ttf",  win_dir);
    snprintf(font_nirmala_b, sizeof(font_nirmala_b), "%s\\Fonts\\NirmalaB.ttf", win_dir);
    snprintf(font_segoe,     sizeof(font_segoe),     "%s\\Fonts\\segoeui.ttf",  win_dir);

    static const ImWchar glyph_ranges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0980, 0x09FF, // Bengali Unicode Block
        0x2000, 0x206F, // General Punctuation (quotes, dashes)
        0x2190, 0x2195, // Arrows
        0x25A0, 0x25FF, // Geometric Shapes
        0,
    };

    ImFontConfig cfg;
    cfg.MergeMode = false;
    cfg.OversampleH = 2;
    cfg.OversampleV = 2;

    // 1. UI Font (15px)
    editor::g_FontUI = io.Fonts->AddFontFromFileTTF(font_nirmala, 15.0f, &cfg, glyph_ranges);
    if (!editor::g_FontUI) {
        editor::g_FontUI = io.Fonts->AddFontDefault();
    }

    // 2. Large Key Font (20px) for Bangla Primary Key Character
    editor::g_FontKeyLarge = io.Fonts->AddFontFromFileTTF(font_nirmala_b, 20.0f, &cfg, glyph_ranges);
    if (!editor::g_FontKeyLarge) {
        editor::g_FontKeyLarge = io.Fonts->AddFontFromFileTTF(font_nirmala, 20.0f, &cfg, glyph_ranges);
        if (!editor::g_FontKeyLarge) editor::g_FontKeyLarge = editor::g_FontUI;
    }

    // 3. Sub Key Font (13px) for Shifted Characters & Modifier Labels
    editor::g_FontKeySub = io.Fonts->AddFontFromFileTTF(font_nirmala, 13.0f, &cfg, glyph_ranges);
    if (!editor::g_FontKeySub) {
        editor::g_FontKeySub = editor::g_FontUI;
    }

    // 4. Small Key Font (11px) for Physical Latin Key Labels (Q, W, etc.)
    editor::g_FontKeySmall = io.Fonts->AddFontFromFileTTF(font_segoe, 11.0f, &cfg, glyph_ranges);
    if (!editor::g_FontKeySmall) {
        editor::g_FontKeySmall = io.Fonts->AddFontFromFileTTF(font_nirmala, 11.0f, &cfg, glyph_ranges);
        if (!editor::g_FontKeySmall) editor::g_FontKeySmall = editor::g_FontUI;
    }

    // Default Style Configuration
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();

    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 8.0f;
    style.FrameRounding     = 6.0f;
    style.PopupRounding     = 8.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding      = 4.0f;
    style.WindowBorderSize  = 0.0f;
    style.FrameBorderSize   = 1.0f;
    style.ItemSpacing       = ImVec2(10.0f, 8.0f);
    style.FramePadding      = ImVec2(8.0f, 6.0f);

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initial state
    MainWindow::Initialize();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // UI Code
        if (MainWindow::Render()) {
            glfwSetWindowShouldClose(window, true);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        // Zero-glare dark clear color
        glClearColor(0.08f, 0.10f, 0.14f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    MainWindow::Shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main(__argc, __argv);
}
#endif

