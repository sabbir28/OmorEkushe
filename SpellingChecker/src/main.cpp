#include <iostream>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "SpellingCheckerUI.h"

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(900, 700, "Bangla Spelling Checker", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Load Unicode Font (Bengali support)
    char font_path[512] = "";
#ifdef _WIN32
    GetWindowsDirectoryA(font_path, 512);
    strcat_s(font_path, sizeof(font_path), "\\Fonts\\Nirmala.ttf");
#endif
    
    ImFontConfig font_config;
    font_config.MergeMode = false;
    static const ImWchar ranges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0980, 0x09FF, // Bengali
        0,
    };
    
    bool fontLoaded = false;
    if (strlen(font_path) > 0) {
        FILE* test_f;
        if (fopen_s(&test_f, font_path, "rb") == 0 && test_f != nullptr) {
            fclose(test_f);
            if (io.Fonts->AddFontFromFileTTF(font_path, 20.0f, &font_config, ranges) != NULL) {
                fontLoaded = true;
            }
        }
    }
    
    if (!fontLoaded) {
        io.Fonts->AddFontDefault();
    }

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initial state
    std::string dictPath;
    if (argc >= 2) {
        dictPath = argv[1];
    } else {
        // Use the compile-time data directory
        dictPath = BIJOY_DATA_DIR;
        dictPath += "ai/unique_words.txt";
    }
    
    bijoy::spelling::SpellingCheckerUI::Initialize(dictPath);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // UI Code
        if (bijoy::spelling::SpellingCheckerUI::Render()) {
            glfwSetWindowShouldClose(window, true);
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.15f, 0.15f, 0.15f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    bijoy::spelling::SpellingCheckerUI::Shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

#if defined(_WIN32) && defined(_MSC_VER)
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;
    return main(__argc, __argv);
}
#endif

