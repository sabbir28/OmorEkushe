cmake_minimum_required(VERSION 3.14)
project(OmorEkushe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


option(BIJOY_BUILD_LAYOUT_EDITOR "Build the LayoutEditor Win32 prototype" ON)

set(BIJOY_BUILD_ROOT "${CMAKE_SOURCE_DIR}/build")
set(BIJOY_RUNTIME_OUTPUT_DIR "${BIJOY_BUILD_ROOT}/bin")
set(BIJOY_DATA_DIR "${BIJOY_BUILD_ROOT}/data")

file(MAKE_DIRECTORY "${BIJOY_RUNTIME_OUTPUT_DIR}")
file(MAKE_DIRECTORY "${BIJOY_DATA_DIR}")
file(MAKE_DIRECTORY "${BIJOY_BUILD_ROOT}/temp")

add_executable(OmorEkushe WIN32
        src/app/main.cpp
        src/core/app_state.cpp
        src/core/keyboard_hook_service.cpp
        src/core/layout.cpp
        src/core/layout_discovery.cpp
        src/core/startup_options.cpp
        src/core/window_layout_binding.cpp
        src/platform/windows/main_window.cpp
        src/platform/windows/main_window/main_window_background.cpp
        src/platform/windows/main_window/main_window_helpers.cpp
        src/platform/windows/main_window/main_window_layout.cpp
        src/platform/windows/main_window/main_window_tray.cpp
        src/platform/windows/native_input.cpp
        src/platform/windows/registration_dialog.cpp
        src/platform/windows/splash_screen.cpp
        src/utils/system_utils.cpp
        )

option(BIJOY_INCLUDE_RESOURCES "Compile Win32 resource file (app.rc)" ON)
if(BIJOY_INCLUDE_RESOURCES)
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
        BIJOY_DATA_DIR=L"${BIJOY_DATA_DIR}/"
)

target_link_libraries(OmorEkushe PRIVATE comctl32 shell32 shlwapi msimg32)

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

if(BIJOY_BUILD_LAYOUT_EDITOR)
    add_subdirectory(LayoutEditor)
endif()


