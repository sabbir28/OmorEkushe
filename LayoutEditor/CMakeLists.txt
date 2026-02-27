cmake_minimum_required(VERSION 3.20)
project(LayoutEditor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(LayoutEditor WIN32
        src/LayoutEditor.cpp
        )

target_include_directories(LayoutEditor PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

target_compile_definitions(LayoutEditor PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        )

target_link_libraries(LayoutEditor PRIVATE
        user32
        gdi32
        comctl32
        )

if(MINGW)
    target_compile_options(LayoutEditor PRIVATE -Wall -Wextra -Wpedantic -municode)
    target_link_options(LayoutEditor PRIVATE
            -municode
            -mwindows
            )
endif()
