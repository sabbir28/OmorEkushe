# cmake/CompilerWarnings.cmake
# Standardized compiler warnings across MSVC, GCC, and Clang.

function(omorekushe_set_compiler_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE
            /W4
            /utf-8
            /permissive-
        )
    else()
        target_compile_options(${target_name} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wno-unused-parameter
        )
    endif()
endfunction()

