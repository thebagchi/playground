include_directories(${CMAKE_SOURCE_DIR})

add_executable(main.bin main.cc)

target_link_libraries(main.bin Boost::json)