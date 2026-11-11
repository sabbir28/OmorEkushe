cmake_minimum_required(VERSION 3.14)
project(SpellingChecker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    ${CMAKE_SOURCE_DIR}/include
)

target_compile_definitions(SpellingChecker PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    OMOREKUSHE_DATA_DIR="${OMOREKUSHE_DATA_DIR}"
    BIJOY_DATA_DIR="${OMOREKUSHE_DATA_DIR}"
)

if(TARGET omorekushe_rust_lib)
    target_compile_definitions(SpellingChecker PRIVATE OMOREKUSHE_ENABLE_RUST=1)
    target_link_libraries(SpellingChecker PRIVATE omorekushe_rust_lib)
    if(WIN32)
        target_link_libraries(SpellingChecker PRIVATE ws2_32 userenv ntdll bcrypt)
    endif()
endif()

target_link_libraries(SpellingChecker PRIVATE
    imgui
    comctl32
    user32
    gdi32
    shlwapi
)

if(COMMAND omorekushe_set_compiler_warnings)
    omorekushe_set_compiler_warnings(SpellingChecker)
endif()

# =========================================================
# Aggregate Build Integration
# =========================================================

if(TARGET omorekushe_all)
    add_dependencies(omorekushe_all SpellingChecker)
endif()
