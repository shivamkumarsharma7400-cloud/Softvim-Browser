# DownloadCEF.cmake
# Downloads and extracts the CEF binary distribution if not already present.
#
# Usage:
#   DownloadCEF("windows64" "131.3.1+..." "/path/to/third_party/cef")

function(DownloadCEF CEF_PLATFORM CEF_VERSION CEF_ROOT)
    # Check if CEF is already downloaded
    if(EXISTS "${CEF_ROOT}/cmake/FindCEF.cmake")
        message(STATUS "CEF binary distribution already present at ${CEF_ROOT}")
        return()
    endif()

    # Build the distribution name and URL
    set(CEF_DISTRIBUTION "cef_binary_${CEF_VERSION}_${CEF_PLATFORM}")

    # URL-encode the '+' characters in the version string
    string(REPLACE "+" "%2B" CEF_VERSION_ENCODED "${CEF_VERSION}")
    set(CEF_DISTRIBUTION_ENCODED "cef_binary_${CEF_VERSION_ENCODED}_${CEF_PLATFORM}")

    set(CEF_DOWNLOAD_URL "https://cef-builds.spotifycdn.com/${CEF_DISTRIBUTION_ENCODED}_minimal.tar.bz2")
    set(CEF_DOWNLOAD_DIR "${CMAKE_BINARY_DIR}/cef_download")
    set(CEF_ARCHIVE "${CEF_DOWNLOAD_DIR}/${CEF_DISTRIBUTION}_minimal.tar.bz2")

    # Create download directory
    file(MAKE_DIRECTORY "${CEF_DOWNLOAD_DIR}")

    # Download the archive
    message(STATUS "")
    message(STATUS "Downloading CEF binary distribution...")
    message(STATUS "  URL: ${CEF_DOWNLOAD_URL}")
    message(STATUS "  To:  ${CEF_ARCHIVE}")
    message(STATUS "  This may take several minutes on first build.")
    message(STATUS "")

    if(NOT EXISTS "${CEF_ARCHIVE}")
        file(DOWNLOAD
            "${CEF_DOWNLOAD_URL}"
            "${CEF_ARCHIVE}"
            SHOW_PROGRESS
            STATUS DOWNLOAD_STATUS
            TLS_VERIFY ON
        )
        list(GET DOWNLOAD_STATUS 0 DOWNLOAD_ERROR_CODE)
        list(GET DOWNLOAD_STATUS 1 DOWNLOAD_ERROR_STRING)
        if(NOT DOWNLOAD_ERROR_CODE EQUAL 0)
            file(REMOVE "${CEF_ARCHIVE}")
            message(FATAL_ERROR
                "Failed to download CEF: ${DOWNLOAD_ERROR_STRING}\n"
                "URL: ${CEF_DOWNLOAD_URL}\n"
                "Please check the CEF version string in CMakeLists.txt.\n"
                "Browse available versions at: https://cef-builds.spotifycdn.com/index.html"
            )
        endif()
    endif()

    # Extract the archive
    message(STATUS "Extracting CEF binary distribution...")
    file(MAKE_DIRECTORY "${CEF_ROOT}")

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xjf "${CEF_ARCHIVE}"
        WORKING_DIRECTORY "${CEF_DOWNLOAD_DIR}"
        RESULT_VARIABLE EXTRACT_RESULT
    )

    if(NOT EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract CEF archive: ${CEF_ARCHIVE}")
    endif()

    # Move extracted contents to CEF_ROOT
    # The archive extracts to a directory named after the distribution
    file(GLOB CEF_EXTRACTED_DIRS "${CEF_DOWNLOAD_DIR}/cef_binary_*")
    list(LENGTH CEF_EXTRACTED_DIRS NUM_DIRS)
    if(NUM_DIRS EQUAL 0)
        message(FATAL_ERROR "CEF extraction produced no output directory.")
    endif()
    list(GET CEF_EXTRACTED_DIRS 0 CEF_EXTRACTED_DIR)

    # Copy contents to the target location
    file(GLOB CEF_CONTENTS "${CEF_EXTRACTED_DIR}/*")
    foreach(ITEM ${CEF_CONTENTS})
        get_filename_component(ITEM_NAME "${ITEM}" NAME)
        if(IS_DIRECTORY "${ITEM}")
            file(COPY "${ITEM}" DESTINATION "${CEF_ROOT}")
        else()
            file(COPY "${ITEM}" DESTINATION "${CEF_ROOT}")
        endif()
    endforeach()

    # Verify extraction was successful
    if(NOT EXISTS "${CEF_ROOT}/cmake/FindCEF.cmake")
        message(FATAL_ERROR
            "CEF extraction did not produce expected files.\n"
            "Expected: ${CEF_ROOT}/cmake/FindCEF.cmake\n"
            "Extracted to: ${CEF_EXTRACTED_DIR}"
        )
    endif()

    message(STATUS "CEF binary distribution ready at ${CEF_ROOT}")
endfunction()

