cmake_minimum_required(VERSION 3.14)
project(Bundler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(bundler main.cpp)

# Ensure the bundler tool also goes to the main build/bin folder
set_target_properties(bundler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIJOY_RUNTIME_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BIJOY_RUNTIME_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BIJOY_RUNTIME_OUTPUT_DIR}"
)
