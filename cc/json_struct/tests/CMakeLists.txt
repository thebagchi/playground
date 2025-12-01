include_directories(${CMAKE_SOURCE_DIR})

add_executable(test_json test_json.cc)

target_link_libraries(test_json Boost::json Boost::unit_test_framework)

add_executable(test_errors test_errors.cc)

target_link_libraries(test_errors Boost::json Boost::unit_test_framework)

add_executable(test_describe test_describe.cc)

target_link_libraries(test_describe Boost::json Boost::unit_test_framework)

add_executable(test_rfc6902 test_rfc6902.cc)

target_link_libraries(test_rfc6902 Boost::json Boost::unit_test_framework)

add_executable(test_benchmark test_benchmark.cc)

target_link_libraries(test_benchmark Boost::json Boost::unit_test_framework)