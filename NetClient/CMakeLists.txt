cmake_minimum_required(VERSION 3.14)
project(NetClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add version management as a pre-build step
# AUDIT FIX: Previously `find_package(Python3 REQUIRED)` caused CMake configuration
# to fail when Python3 was not installed on the system (e.g., standard C++ Visual Studio environments).
# Made Python3 optional (QUIET) and only run `version_manager.py` if the interpreter is present.
find_package(Python3 COMPONENTS Interpreter QUIET)

if(Python3_FOUND)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/NetClient.rc" "${CMAKE_CURRENT_SOURCE_DIR}/include/NetClientVersion.h"
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/version_manager.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Updating NetClient version and metadata"
        VERBATIM
    )
else()
    message(STATUS "Python3 interpreter not found: skipping automatic version bump. Using existing NetClient.rc and NetClientVersion.h.")
endif()

set(NETCLIENT_SOURCES
    src/NetClient.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/src/NetClient.rc"
)

add_library(NetClient SHARED ${NETCLIENT_SOURCES})

target_include_directories(NetClient PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_definitions(NetClient PRIVATE NETCLIENT_EXPORTS)

if(WIN32)
    enable_language(RC)
    target_link_libraries(NetClient PRIVATE winhttp)
endif()

# Distribution details for other developers
set(SDK_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build")

# Set output properties
set_target_properties(NetClient PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${SDK_OUTPUT_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${SDK_OUTPUT_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${SDK_OUTPUT_DIR}/lib"
)

# Custom command to copy headers and documentation to SDK folder
add_custom_command(TARGET NetClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SDK_OUTPUT_DIR}/include"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${CMAKE_CURRENT_SOURCE_DIR}/include/NetClient.h" 
        "${SDK_OUTPUT_DIR}/include/NetClient.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${CMAKE_CURRENT_SOURCE_DIR}/include/NetClientVersion.h" 
        "${SDK_OUTPUT_DIR}/include/NetClientVersion.h"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
        "${CMAKE_CURRENT_SOURCE_DIR}/README_SDK.md" 
        "${SDK_OUTPUT_DIR}/README_SDK.md"
    COMMENT "Copying NetClient headers and docs to SDK folder"
)
