cmake_minimum_required(VERSION 3.14)

project(json_struct)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include the boost download function
include(cmake/boost.cmake)

# Download Boost
set(BOOST_VERSION "1.89.0")
set(BOOST_DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/thirdparty/boost")
download_boost(${BOOST_VERSION} ${BOOST_DOWNLOAD_DIR})

# Install Boost
set(BOOST_SOURCE_DIR "${BOOST_DOWNLOAD_DIR}/boost-${BOOST_VERSION}")
set(BOOST_INSTALL_DIR "${CMAKE_SOURCE_DIR}/libs/boost")
install_boost(${BOOST_SOURCE_DIR} ${BOOST_INSTALL_DIR})

# Set the path to the installed Boost
set(CMAKE_PREFIX_PATH "${BOOST_INSTALL_DIR}")

# Prefer static linking for Boost
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_DEBUG_LIBS OFF)
set(Boost_USE_RELEASE_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_MULTITHREADED ON)

# Enable fully static linking
set(CMAKE_EXE_LINKER_FLAGS "-static")

find_package(Boost REQUIRED COMPONENTS json unit_test_framework)

add_executable(main.bin main.cc)

target_link_libraries(main.bin Boost::json)

add_executable(test_json test_json.cc)

target_link_libraries(test_json Boost::json Boost::unit_test_framework)

# Custom target to remove Boost
add_custom_target(remove-boost
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${BOOST_DOWNLOAD_DIR}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${BOOST_INSTALL_DIR}
    COMMENT "Removing downloaded and installed Boost"
)