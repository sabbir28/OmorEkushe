# tests/CMakeLists.txt
# Automated Unit & Integration Tests for OmorEkushe

add_executable(PhoneticTests
    test_phonetic_parser.cpp
    ${CMAKE_SOURCE_DIR}/src/core/phonetic_data.cpp
    ${CMAKE_SOURCE_DIR}/src/core/phonetic_parser.cpp
    ${CMAKE_SOURCE_DIR}/src/core/phonetic_engine.cpp
    ${CMAKE_SOURCE_DIR}/src/core/layout.cpp
    ${CMAKE_SOURCE_DIR}/src/core/startup_options.cpp
)

target_include_directories(PhoneticTests PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(WIN32)
    target_sources(PhoneticTests PRIVATE ${CMAKE_SOURCE_DIR}/src/platform/windows/platform_input_win.cpp)
    target_link_libraries(PhoneticTests PRIVATE advapi32 user32)
elseif(APPLE)
    target_sources(PhoneticTests PRIVATE ${CMAKE_SOURCE_DIR}/src/platform/macos/platform_input_macos.mm)
    target_link_libraries(PhoneticTests PRIVATE ${COREGRAPHICS_LIB} ${COCOA_LIB})
else()
    target_sources(PhoneticTests PRIVATE
        ${CMAKE_SOURCE_DIR}/src/platform/linux/platform_input_linux.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/linux/x11_input_method.cpp
        ${CMAKE_SOURCE_DIR}/src/platform/linux/wayland_input_method.cpp
    )
    if(X11_FOUND)
        target_include_directories(PhoneticTests PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(PhoneticTests PRIVATE ${X11_LIBRARIES})
        if(XTST_LIB)
            target_link_libraries(PhoneticTests PRIVATE ${XTST_LIB})
        endif()
    endif()
    target_link_libraries(PhoneticTests PRIVATE pthread dl)
endif()

target_compile_definitions(PhoneticTests PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
)

omorekushe_set_compiler_warnings(PhoneticTests)

if(TARGET omorekushe_rust_lib)
    target_compile_definitions(PhoneticTests PRIVATE OMOREKUSHE_ENABLE_RUST=1)
    target_link_libraries(PhoneticTests PRIVATE omorekushe_rust_lib)
    if(WIN32)
        target_link_libraries(PhoneticTests PRIVATE ws2_32 userenv ntdll bcrypt)
    endif()
endif()

add_test(NAME PhoneticParserTests COMMAND PhoneticTests)

