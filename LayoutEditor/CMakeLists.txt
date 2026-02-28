project(LayoutEditor LANGUAGES CXX)

add_executable(LayoutEditor WIN32
        src/main.cpp
        src/MainWindow.cpp
)

target_include_directories(LayoutEditor PRIVATE include)
target_compile_definitions(LayoutEditor PRIVATE UNICODE _UNICODE)
target_link_libraries(LayoutEditor PRIVATE comctl32 user32 gdi32)

set_target_properties(LayoutEditor PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${BIJOY_RUNTIME_OUTPUT_DIR}"
)

# Also add to the omorekushe_all target if defined
if(TARGET omorekushe_all)
    add_dependencies(omorekushe_all LayoutEditor)
endif()
