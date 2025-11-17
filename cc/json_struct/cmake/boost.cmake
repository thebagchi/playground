function(download_boost VERSION DOWNLOAD_DIR)
    # Check if Boost is already extracted
    set(EXTRACTED_DIR "${DOWNLOAD_DIR}/boost-${VERSION}")
    if(EXISTS ${EXTRACTED_DIR})
        message(STATUS "Boost ${VERSION} already extracted to ${EXTRACTED_DIR}")
        return()
    endif()

    # Construct the download URL
    set(URL "https://github.com/boostorg/boost/releases/download/boost-${VERSION}/boost-${VERSION}-cmake.tar.gz")

    # Set the archive path
    set(ARCHIVE_PATH "${DOWNLOAD_DIR}/boost-${VERSION}-cmake.tar.gz")

    # Download the archive
    file(DOWNLOAD ${URL} ${ARCHIVE_PATH}
         SHOW_PROGRESS
         STATUS DOWNLOAD_STATUS
         LOG DOWNLOAD_LOG)

    # Check if download was successful
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if(NOT STATUS_CODE EQUAL 0)
        message(FATAL_ERROR "Failed to download Boost ${VERSION}: ${DOWNLOAD_LOG}")
    endif()

    # Extract the archive
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar xzf ${ARCHIVE_PATH}
        WORKING_DIRECTORY ${DOWNLOAD_DIR}
        RESULT_VARIABLE EXTRACT_RESULT
    )

    if(NOT EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract Boost ${VERSION} archive")
    endif()

    message(STATUS "Boost ${VERSION} downloaded and extracted to ${DOWNLOAD_DIR}")
endfunction()

function(install_boost SOURCE_DIR INSTALL_DIR)
    # Check if Boost is already installed
    if(EXISTS ${INSTALL_DIR}/include/boost/version.hpp)
        message(STATUS "Boost already installed to ${INSTALL_DIR}")
        return()
    endif()

    # Configure Boost with CMake
    execute_process(
        COMMAND ${CMAKE_COMMAND} -S ${SOURCE_DIR} -B ${SOURCE_DIR}/build
        -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
        -DCMAKE_BUILD_TYPE=Release
        RESULT_VARIABLE CONFIG_RESULT
    )

    if(NOT CONFIG_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to configure Boost in ${SOURCE_DIR}")
    endif()

    # Build and install Boost
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${SOURCE_DIR}/build --target install
        RESULT_VARIABLE BUILD_RESULT
    )

    if(NOT BUILD_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to build and install Boost to ${INSTALL_DIR}")
    endif()

    message(STATUS "Boost installed to ${INSTALL_DIR}")
endfunction()