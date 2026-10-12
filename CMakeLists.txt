cmake_minimum_required(VERSION 3.14)
project(OmorEkushe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BIJOY_BUILD_LAYOUT_EDITOR "Build the LayoutEditor GUI (GLFW + OpenGL + Dear ImGui)" ON)

set(BIJOY_BUILD_ROOT "${CMAKE_SOURCE_DIR}/build")
set(BIJOY_RUNTIME_OUTPUT_DIR "${BIJOY_BUILD_ROOT}/bin")
set(BIJOY_DATA_DIR "${BIJOY_BUILD_ROOT}/data")
set(FETCHCONTENT_BASE_DIR "${BIJOY_BUILD_ROOT}/_deps" CACHE PATH "Dependencies Base Directory")

file(MAKE_DIRECTORY "${BIJOY_RUNTIME_OUTPUT_DIR}")
file(MAKE_DIRECTORY "${BIJOY_DATA_DIR}")
file(MAKE_DIRECTORY "${BIJOY_BUILD_ROOT}/temp")
file(MAKE_DIRECTORY "${BIJOY_BUILD_ROOT}/_deps")

if(EXISTS "${CMAKE_SOURCE_DIR}/data")
    file(COPY "${CMAKE_SOURCE_DIR}/data/" DESTINATION "${BIJOY_DATA_DIR}")
endif()

# =========================================================
# Main Executable: OmorEkushe (Cross-Platform Native Engine)
# =========================================================
if(WIN32)
    add_executable(OmorEkushe WIN32)
    target_sources(OmorEkushe PRIVATE
        src/platform/windows/platform_input_win.cpp
        src/platform/windows/main_window.cpp
        src/platform/windows/main_window/main_window_background.cpp
        src/platform/windows/main_window/main_window_helpers.cpp
        src/platform/windows/main_window/main_window_layout.cpp
        src/platform/windows/main_window/main_window_tray.cpp
        src/platform/windows/native_input.cpp
        src/platform/windows/registration_dialog.cpp
        src/platform/windows/splash_screen.cpp
        src/platform/windows/options_overlay.cpp
        src/platform/windows/options_dialog.cpp
    )
elseif(APPLE)
    add_executable(OmorEkushe MACOSX_BUNDLE)
    target_sources(OmorEkushe PRIVATE
        src/platform/macos/platform_input_macos.mm
        src/platform/macos/imk_input_controller.mm
        src/platform/macos/main_window.mm
    )
    find_library(COCOA_LIB Cocoa REQUIRED)
    find_library(IMK_LIB InputMethodKit REQUIRED)
    find_library(CARBON_LIB Carbon REQUIRED)
    find_library(COREGRAPHICS_LIB CoreGraphics REQUIRED)
    target_link_libraries(OmorEkushe PRIVATE ${COCOA_LIB} ${IMK_LIB} ${CARBON_LIB} ${COREGRAPHICS_LIB})
elseif(UNIX AND NOT APPLE)
    add_executable(OmorEkushe)
    target_sources(OmorEkushe PRIVATE
        src/platform/linux/platform_input_linux.cpp
        src/platform/linux/x11_input_method.cpp
        src/platform/linux/wayland_input_method.cpp
        src/platform/linux/main_window.cpp
        src/platform/linux/native_input.cpp
        src/platform/linux/registration_dialog.cpp
        src/platform/linux/splash_screen.cpp
    )
    find_package(X11)
    if(X11_FOUND)
        target_include_directories(OmorEkushe PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(OmorEkushe PRIVATE ${X11_LIBRARIES})
        find_library(XTST_LIB Xtst)
        if(XTST_LIB)
            target_link_libraries(OmorEkushe PRIVATE ${XTST_LIB})
        endif()
    endif()
    target_link_libraries(OmorEkushe PRIVATE pthread dl)
endif()

target_sources(OmorEkushe PRIVATE
        src/app/main.cpp
        src/app/application.cpp
        src/error/crash_handler.cpp
        src/core/app_state.cpp
        src/core/keyboard_hook_service.cpp
        src/core/layout.cpp
        src/core/layout_discovery.cpp
        src/core/phonetic_data.cpp
        src/core/phonetic_parser.cpp
        src/core/phonetic_engine.cpp
        src/core/startup_options.cpp
        src/core/window_layout_binding.cpp
        src/utils/system_utils.cpp
)

option(BIJOY_INCLUDE_RESOURCES "Compile Win32 resource file (app.rc)" ON)
if(WIN32 AND BIJOY_INCLUDE_RESOURCES)
  if(CMAKE_RC_COMPILER)
    enable_language(RC)
    target_sources(OmorEkushe PRIVATE src/platform/windows/app.rc)
  else()
    message(WARNING "Resource compiler not found. Building without src/platform/windows/app.rc")
  endif()
endif()

target_include_directories(OmorEkushe PRIVATE include)
target_compile_definitions(
        OmorEkushe
        PRIVATE
        UNICODE
        _UNICODE
        NOMINMAX
        BIJOY_DATA_DIR=L"${BIJOY_DATA_DIR}/"
)

if(WIN32)
    target_link_libraries(OmorEkushe PRIVATE comctl32 shell32 shlwapi msimg32 dbghelp user32 gdi32 advapi32)
endif()

set_target_properties(OmorEkushe PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${BIJOY_RUNTIME_OUTPUT_DIR}"
)

if(MSVC)
  target_compile_options(OmorEkushe PRIVATE /W4)
else()
  target_compile_options(OmorEkushe PRIVATE -Wall -Wextra)
endif()

# ------------------------------------------------------------------
# MinGW runtime DLL copy fix (libstdc++-6.dll, libgcc, winpthread)
# ------------------------------------------------------------------
if(MINGW)
  get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

  set(MINGW_DLLS
          libstdc++-6.dll
          libgcc_s_seh-1.dll
          libgcc_s_dw2-1.dll
          libwinpthread-1.dll
          )

  foreach(DLL ${MINGW_DLLS})
    if(EXISTS "${MINGW_BIN_DIR}/${DLL}")
      add_custom_command(
              TARGET OmorEkushe POST_BUILD
              COMMAND ${CMAKE_COMMAND} -E copy_if_different
              "${MINGW_BIN_DIR}/${DLL}"
              "$<TARGET_FILE_DIR:OmorEkushe>"
      )
    endif()
  endforeach()
endif()

add_custom_target(omorekushe_all
        DEPENDS OmorEkushe
)

add_executable(PhoneticTests
    src/core/phonetic_data.cpp
    src/core/phonetic_parser.cpp
    src/core/phonetic_engine.cpp
    src/core/layout.cpp
    src/core/startup_options.cpp
    src/core/test_phonetic_parser.cpp
)
target_include_directories(PhoneticTests PRIVATE include)

if(WIN32)
    target_sources(PhoneticTests PRIVATE src/platform/windows/platform_input_win.cpp)
    target_link_libraries(PhoneticTests PRIVATE advapi32 user32)
elseif(APPLE)
    target_sources(PhoneticTests PRIVATE src/platform/macos/platform_input_macos.mm)
    target_link_libraries(PhoneticTests PRIVATE ${COREGRAPHICS_LIB} ${COCOA_LIB})
else()
    target_sources(PhoneticTests PRIVATE
        src/platform/linux/platform_input_linux.cpp
        src/platform/linux/x11_input_method.cpp
        src/platform/linux/wayland_input_method.cpp
    )
    target_link_libraries(PhoneticTests PRIVATE pthread dl)
endif()

set_target_properties(PhoneticTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIJOY_RUNTIME_OUTPUT_DIR}"
)

# Optional Layout Editor dependencies (only if explicitly enabled on Windows)
if(BIJOY_BUILD_LAYOUT_EDITOR AND WIN32)
    include(FetchContent)
    set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps CACHE PATH "Dependencies Base Directory")
    set(FETCHCONTENT_QUIET OFF)

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    # AUDIT FIX: Disable GLFW_INSTALL to prevent configure_package_config_file failures during subproject builds.
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/_deps/glfw-build/src")
    file(MAKE_DIRECTORY "${FETCHCONTENT_BASE_DIR}/glfw-build/src")
    FetchContent_MakeAvailable(glfw)

    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
    )
    FetchContent_GetProperties(imgui)
    if(NOT imgui_POPULATED)
        FetchContent_Populate(imgui)
    endif()

    find_package(OpenGL REQUIRED)

    if(NOT TARGET imgui)
        add_library(imgui STATIC
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )
        target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
        target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD=0)
        target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
    endif()

    add_subdirectory(LayoutEditor)
    add_subdirectory(SpellingChecker)
endif()

# Add NetClient (Windows)
if(WIN32)
    add_subdirectory(NetClient)
endif()

