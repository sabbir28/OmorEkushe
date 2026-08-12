cmake_minimum_required(VERSION 3.14)
project(SpellingChecker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =========================================================
# Dependencies (reused from root CMakeLists.txt)
# =========================================================

# Dependencies for GLFW and ImGui are already included at the root level!


# =========================================================
# SpellingChecker Application
# =========================================================

add_executable(SpellingChecker WIN32
    src/main.cpp
    src/SpellingChecker.cpp
    src/SpellingCheckerUI.cpp
)

target_include_directories(SpellingChecker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(SpellingChecker PRIVATE
    UNICODE
    _UNICODE
    BIJOY_DATA_DIR="${BIJOY_DATA_DIR}/"
)

target_link_libraries(SpellingChecker PRIVATE
    imgui
    comctl32
    user32
    gdi32
    shlwapi
)

# =========================================================
# Output Directory
# =========================================================

set(SPELLING_CHECKER_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/bin")

set_target_properties(SpellingChecker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SPELLING_CHECKER_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${SPELLING_CHECKER_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${SPELLING_CHECKER_OUTPUT_DIR}"
)

if(MSVC)
    target_compile_options(SpellingChecker PRIVATE /W4 /utf-8)
else()
    target_compile_options(SpellingChecker PRIVATE -Wall -Wextra)
endif()

# =========================================================
# Aggregate Build
# =========================================================

if(TARGET omorekushe_all)
    add_dependencies(omorekushe_all SpellingChecker)
endif()
