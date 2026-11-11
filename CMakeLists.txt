# Copyright (c) 2024 Softvim Solutions. All rights reserved.
# Softvim Browser — Root CMake Configuration
#
# This project builds a Chromium-based browser using CEF (Chromium Embedded Framework).
# CEF is downloaded automatically during CMake configuration.

cmake_minimum_required(VERSION 3.21)
project(SoftvimBrowser VERSION 0.1.0 LANGUAGES CXX)

# ──────────────────────────────────────────────
# CEF Configuration
# ──────────────────────────────────────────────

# CEF version to download. Update this to upgrade Chromium.
# Find versions at: https://cef-builds.spotifycdn.com/index.html
set(CEF_VERSION "144.0.6+g5f7e671+chromium-144.0.7559.59"
    CACHE STRING "CEF binary distribution version")

# Platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CEF_PLATFORM "windows64")
    else()
        set(CEF_PLATFORM "windows32")
    endif()
else()
    message(FATAL_ERROR "Softvim Browser currently supports Windows only.")
endif()

# Sandbox is disabled for development simplicity.
# Can be re-enabled for release builds.
option(USE_SANDBOX "Enable CEF sandbox" OFF)

# ──────────────────────────────────────────────
# Download CEF Binary Distribution
# ──────────────────────────────────────────────

set(CEF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cef")

# Include our download helper
include(cmake/DownloadCEF.cmake)
DownloadCEF("${CEF_PLATFORM}" "${CEF_VERSION}" "${CEF_ROOT}")

# ──────────────────────────────────────────────
# CEF CMake Integration
# ──────────────────────────────────────────────

# Add CEF's own cmake modules to the module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

# Load CEF variables and macros
find_package(CEF REQUIRED)

# ──────────────────────────────────────────────
# Compiler Settings
# ──────────────────────────────────────────────

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC specific settings
if(MSVC)
    # Use multi-threaded static runtime (required by CEF)
    set(CEF_RUNTIME_LIBRARY_FLAG "/MT" CACHE STRING "")
    # Suppress some common warnings
    add_compile_options(/W3 /wd4100 /wd4244 /wd4267)
    # UTF-8 source encoding
    add_compile_options(/utf-8)
endif()

# ──────────────────────────────────────────────
# Build libcef_dll_wrapper (required static library)
# ──────────────────────────────────────────────

add_subdirectory(${CEF_ROOT}/libcef_dll ${CMAKE_BINARY_DIR}/libcef_dll_wrapper)

# ──────────────────────────────────────────────
# Build Softvim Browser
# ──────────────────────────────────────────────

add_subdirectory(src)

# ──────────────────────────────────────────────
# Print configuration summary
# ──────────────────────────────────────────────

message(STATUS "")
message(STATUS "╔══════════════════════════════════════════╗")
message(STATUS "║     Softvim Browser Build Config         ║")
message(STATUS "╠══════════════════════════════════════════╣")
message(STATUS "║  Version:    ${PROJECT_VERSION}                      ║")
message(STATUS "║  Platform:   ${CEF_PLATFORM}                   ║")
message(STATUS "║  CEF:        ${CEF_VERSION}")
message(STATUS "║  Sandbox:    ${USE_SANDBOX}                         ║")
message(STATUS "║  Build type: ${CMAKE_BUILD_TYPE}                       ║")
message(STATUS "╚══════════════════════════════════════════╝")
message(STATUS "")
