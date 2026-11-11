cmake_minimum_required(VERSION 3.14)
project(NetClient LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional version management pre-build step
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

if(COMMAND omorekushe_set_compiler_warnings)
    omorekushe_set_compiler_warnings(NetClient)
endif()
