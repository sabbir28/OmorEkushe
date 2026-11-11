cmake_minimum_required(VERSION 3.24)
project(LayoutEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================================================
# Main Application
# =========================================================

add_executable(LayoutEditor WIN32
    src/main.cpp
    src/MainWindow.cpp
    src/LayoutData.cpp
    src/KeyboardView.cpp
    src/KeyEditDialog.cpp
    src/Utf8Utils.cpp
)

target_include_directories(LayoutEditor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(LayoutEditor PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
)

target_link_libraries(LayoutEditor PRIVATE
    imgui
    comctl32
    user32
    gdi32
    shlwapi
    dwmapi
)

if(COMMAND omorekushe_set_compiler_warnings)
    omorekushe_set_compiler_warnings(LayoutEditor)
endif()

# =========================================================
# Aggregate Build Integration
# =========================================================

if(TARGET omorekushe_all)
    add_dependencies(omorekushe_all LayoutEditor)
endif()
