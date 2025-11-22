include_directories(${CMAKE_SOURCE_DIR})

add_executable(test_json test_json.cc)

target_link_libraries(test_json Boost::json Boost::unit_test_framework)