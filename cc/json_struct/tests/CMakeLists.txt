include_directories(${CMAKE_SOURCE_DIR})

file(GLOB TEST_SOURCES "*.cc")

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    set(TEST_BIN_NAME "${TEST_NAME}.bin")
    message(STATUS "building test: ${TEST_BIN_NAME}")
    add_executable(${TEST_BIN_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_BIN_NAME} Boost::json Boost::unit_test_framework)
endforeach()