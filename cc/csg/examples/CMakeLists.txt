cmake_minimum_required(VERSION 3.10)
project(CSG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "*.cc")

foreach(SOURCE ${SOURCES})
    get_filename_component(BIN_NAME ${SOURCE} NAME_WE)
    set(BIN_NAME "${BIN_NAME}.bin")
    message(STATUS "building: ${BIN_NAME}")
    add_executable(${BIN_NAME} ${SOURCE})
endforeach()
