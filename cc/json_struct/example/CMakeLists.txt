add_executable(main.bin main.cc)

target_include_directories(main.bin PRIVATE 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

target_link_libraries(main.bin Boost::json)

add_dependencies(main.bin combine_headers)