# Softvim Browser — Source CMakeLists.txt
# Builds the main browser executable and subprocess helper.

set(SOFTVIM_TARGET "SoftvimBrowser")
set(SOFTVIM_HELPER "SoftvimHelper")

# ──────────────────────────────────────────────
# Source Files
# ──────────────────────────────────────────────

set(SOFTVIM_SOURCES
    main.cc
    app.h
    app.cc
    browser_window.h
    browser_window.cc
    client_handler.h
    client_handler.cc
    ui_handler.h
    ui_handler.cc
    tab_manager.h
    tab_manager.cc
    storage_manager.h
    storage_manager.cc
    resource_util.h
    resource_util.cc
)

set(SOFTVIM_HELPER_SOURCES
    subprocess_main.cc
    app.h
)

# ──────────────────────────────────────────────
# Resources
# ──────────────────────────────────────────────

set(SOFTVIM_RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/softvim.rc
)

# UI resources to embed / copy
set(SOFTVIM_UI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../resources/ui")

# ──────────────────────────────────────────────
# Main Browser Executable
# ──────────────────────────────────────────────

add_executable(${SOFTVIM_TARGET} WIN32
    ${SOFTVIM_SOURCES}
    ${SOFTVIM_RESOURCES}
)

# Include paths
target_include_directories(${SOFTVIM_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CEF_ROOT}
)

# Link libraries
target_link_libraries(${SOFTVIM_TARGET} PRIVATE
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
)

# Link against the CEF import library
target_link_libraries(${SOFTVIM_TARGET} PRIVATE
    "${CEF_ROOT}/$<CONFIG>/libcef.lib"
)

# Additional Windows libraries
target_link_libraries(${SOFTVIM_TARGET} PRIVATE
    comctl32
    shlwapi
    rpcrt4
    dwmapi
)

# Compile definitions
target_compile_definitions(${SOFTVIM_TARGET} PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

if(USE_SANDBOX)
    target_compile_definitions(${SOFTVIM_TARGET} PRIVATE CEF_USE_SANDBOX)
    target_link_libraries(${SOFTVIM_TARGET} PRIVATE
        "${CEF_ROOT}/$<CONFIG>/cef_sandbox.lib"
    )
endif()

# ──────────────────────────────────────────────
# Subprocess Helper
# ──────────────────────────────────────────────

add_executable(${SOFTVIM_HELPER} WIN32
    ${SOFTVIM_HELPER_SOURCES}
)

target_include_directories(${SOFTVIM_HELPER} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CEF_ROOT}
)

target_link_libraries(${SOFTVIM_HELPER} PRIVATE
    libcef_dll_wrapper
    "${CEF_ROOT}/$<CONFIG>/libcef.lib"
    ${CEF_STANDARD_LIBS}
)

target_compile_definitions(${SOFTVIM_HELPER} PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# ──────────────────────────────────────────────
# Post-Build: Copy CEF binaries and resources
# ──────────────────────────────────────────────

# Output directory for the browser
set(SOFTVIM_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

# Set output directories
set_target_properties(${SOFTVIM_TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SOFTVIM_OUTPUT_DIR}"
)
set_target_properties(${SOFTVIM_HELPER} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SOFTVIM_OUTPUT_DIR}"
)

# Copy CEF binary files (libcef.dll, etc.)
add_custom_command(TARGET ${SOFTVIM_TARGET} POST_BUILD
    # Copy Release or Debug binaries
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CEF_ROOT}/$<CONFIG>"
        "${SOFTVIM_OUTPUT_DIR}"
    # Copy CEF resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CEF_ROOT}/Resources"
        "${SOFTVIM_OUTPUT_DIR}"
    # Copy our UI resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${SOFTVIM_UI_DIR}"
        "${SOFTVIM_OUTPUT_DIR}/ui"
    COMMENT "Copying CEF binaries, resources, and UI files..."
)

