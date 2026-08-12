cmake_minimum_required(VERSION 3.24)

project(LayoutEditor LANGUAGES CXX)

# =========================================================
# C++ Configuration
# =========================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================================================
# FetchContent
# =========================================================

include(FetchContent)

set(FETCHCONTENT_BASE_DIR
        ${CMAKE_BINARY_DIR}/_deps
        CACHE PATH "Dependencies Base Directory"
)

set(FETCHCONTENT_QUIET OFF)

# =========================================================
# Git / Proxy / VPN Support
# =========================================================

find_program(GIT_EXECUTABLE git REQUIRED)

message(STATUS "Git Executable:")
message(STATUS "${GIT_EXECUTABLE}")

# ---------------------------------------------------------
# Use system proxy automatically
# ---------------------------------------------------------

if(DEFINED ENV{HTTP_PROXY})
    set(ENV{http_proxy} $ENV{HTTP_PROXY})
    message(STATUS "HTTP_PROXY detected")
endif()

if(DEFINED ENV{HTTPS_PROXY})
    set(ENV{https_proxy} $ENV{HTTPS_PROXY})
    message(STATUS "HTTPS_PROXY detected")
endif()

# ---------------------------------------------------------
# Git proxy support
# ---------------------------------------------------------

execute_process(
        COMMAND ${GIT_EXECUTABLE} config --global --get http.proxy
        OUTPUT_VARIABLE GIT_HTTP_PROXY
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
        COMMAND ${GIT_EXECUTABLE} config --global --get https.proxy
        OUTPUT_VARIABLE GIT_HTTPS_PROXY
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GIT_HTTP_PROXY)
    message(STATUS "Git HTTP Proxy:")
    message(STATUS "${GIT_HTTP_PROXY}")
endif()

if(GIT_HTTPS_PROXY)
    message(STATUS "Git HTTPS Proxy:")
    message(STATUS "${GIT_HTTPS_PROXY}")
endif()

# ---------------------------------------------------------
# Better Git network reliability
# ---------------------------------------------------------

set(CMAKE_TLS_VERIFY ON)

set(CMAKE_NETRC OFF)

# =========================================================
# Status
# =========================================================

message(STATUS "==========================================")
message(STATUS "Dependency Directory:")
message(STATUS "${FETCHCONTENT_BASE_DIR}")
message(STATUS "==========================================")

# =========================================================
# Dependencies
# =========================================================
# GLFW and ImGui are already fetched and built in the root CMakeLists.txt


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
        include
)

target_compile_definitions(LayoutEditor PRIVATE
        UNICODE
        _UNICODE
)

target_link_libraries(LayoutEditor PRIVATE
        imgui
        comctl32
        user32
        gdi32
        shlwapi
)

# =========================================================
# Executable Output (Aligned with root project)
# =========================================================

set(LAYOUT_EDITOR_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/bin")

set_target_properties(LayoutEditor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY
        "${LAYOUT_EDITOR_OUTPUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${LAYOUT_EDITOR_OUTPUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${LAYOUT_EDITOR_OUTPUT_DIR}"
)

# =========================================================
# Aggregate Build
# =========================================================

if(TARGET omorekushe_all)
    add_dependencies(omorekushe_all LayoutEditor)
endif()

# =========================================================
# Final Status
# =========================================================

message(STATUS "==========================================")
message(STATUS "Configuration Complete")
message(STATUS "")
message(STATUS "Executable:")
message(STATUS "${CMAKE_BINARY_DIR}/bin")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "${CMAKE_BINARY_DIR}/_deps")
message(STATUS "==========================================")