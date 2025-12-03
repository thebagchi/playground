# CMake script to generate json_all.h from individual header files
# This combines all .h files (except json_all.h) into a single header
# All file paths are passed as command line arguments

# Validate required arguments
if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR must be defined")
endif()
if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE must be defined")
endif()

# Find all header files in the source directory (except json_all.h)
file(GLOB ALL_HEADERS "${SOURCE_DIR}/*.h")
list(REMOVE_ITEM ALL_HEADERS "${OUTPUT_FILE}")

# Build dependency graph by checking #include statements
set(DEPENDENCIES)
foreach(HEADER ${ALL_HEADERS})
    get_filename_component(HEADER_NAME ${HEADER} NAME)
    file(READ "${HEADER}" HEADER_CONTENT)
    
    # Find all local includes in this header
    string(REGEX MATCHALL "#include \"([^\"]+)\"" INCLUDES "${HEADER_CONTENT}")
    
    set(DEPS "")
    foreach(INCLUDE ${INCLUDES})
        string(REGEX REPLACE "#include \"([^\"]+)\"" "\\1" INCLUDE_FILE "${INCLUDE}")
        list(APPEND DEPS "${INCLUDE_FILE}")
    endforeach()
    
    list(APPEND DEPENDENCIES "${HEADER_NAME}:${DEPS}")
endforeach()

# Topological sort to determine dependency order
set(ORDERED_HEADERS)
set(REMAINING_HEADERS ${ALL_HEADERS})

# Iterate until all headers are ordered
while(REMAINING_HEADERS)
    set(FOUND_ONE FALSE)
    
    foreach(HEADER ${REMAINING_HEADERS})
        get_filename_component(HEADER_NAME ${HEADER} NAME)
        
        # Check if this header has no unresolved dependencies
        set(HAS_UNRESOLVED FALSE)
        foreach(DEP_ENTRY ${DEPENDENCIES})
            string(REGEX MATCH "^${HEADER_NAME}:(.*)$" MATCH "${DEP_ENTRY}")
            if(MATCH)
                string(REGEX REPLACE "^${HEADER_NAME}:" "" DEPS "${DEP_ENTRY}")
                if(DEPS)
                    string(REPLACE ";" "|" DEPS "${DEPS}")
                    foreach(REMAINING ${REMAINING_HEADERS})
                        get_filename_component(REM_NAME ${REMAINING} NAME)
                        if(NOT REM_NAME STREQUAL HEADER_NAME)
                            string(REGEX MATCH "${REM_NAME}" MATCH "${DEPS}")
                            if(MATCH)
                                set(HAS_UNRESOLVED TRUE)
                                break()
                            endif()
                        endif()
                    endforeach()
                endif()
                break()
            endif()
        endforeach()
        
        # If no unresolved dependencies, add to ordered list
        if(NOT HAS_UNRESOLVED)
            list(APPEND ORDERED_HEADERS ${HEADER})
            list(REMOVE_ITEM REMAINING_HEADERS ${HEADER})
            set(FOUND_ONE TRUE)
            break()
        endif()
    endforeach()
    
    # Prevent infinite loop in case of circular dependencies
    if(NOT FOUND_ONE)
        message(WARNING "Circular dependency detected or unresolvable dependencies. Using remaining headers in alphabetical order.")
        list(SORT REMAINING_HEADERS)
        list(APPEND ORDERED_HEADERS ${REMAINING_HEADERS})
        break()
    endif()
endwhile()

# Read all header files
set(ALL_CONTENT "")
foreach(HEADER ${ORDERED_HEADERS})
    file(READ "${HEADER}" HEADER_CONTENT)
    get_filename_component(HEADER_NAME ${HEADER} NAME)
    
    # Remove include guards and trailing endif
    # Match pattern: #ifndef GUARD_NAME followed optionally by #define GUARD_NAME VALUE
    # string(REGEX REPLACE "#ifndef [A-Z0-9_]+\n(#define [A-Z0-9_]+ [A-Z0-9_]*\n)?" "" HEADER_CONTENT "${HEADER_CONTENT}")
    # Remove orphaned #endif at end of file (they belong to the removed guards)
    # string(REGEX REPLACE "\n#endif // [A-Z0-9_]+\n*$" "" HEADER_CONTENT "${HEADER_CONTENT}")
    # Remove any remaining orphaned #endif from guard removal
    # string(REGEX REPLACE "\n#endif\n*$" "" HEADER_CONTENT "${HEADER_CONTENT}")
    
    # Remove standard includes and local includes
    string(REGEX REPLACE "#include <[^>]+>\n" "" HEADER_CONTENT "${HEADER_CONTENT}")
    string(REGEX REPLACE "#include \"[^\"]+\"\n" "" HEADER_CONTENT "${HEADER_CONTENT}")
    
    set(ALL_CONTENT "${ALL_CONTENT}\n// =============================================================================\n")
    set(ALL_CONTENT "${ALL_CONTENT}// BEGIN: ${HEADER_NAME}\n")
    set(ALL_CONTENT "${ALL_CONTENT}// =============================================================================\n\n")
    
    # Add base64 namespace alias before json.h content
    if(HEADER_NAME STREQUAL "json.h")
        # Remove existing base64 namespace alias if present
        string(REGEX REPLACE "namespace base64 = boost::beast::detail::base64;\n*" "" HEADER_CONTENT "${HEADER_CONTENT}")
        set(ALL_CONTENT "${ALL_CONTENT}namespace base64 = boost::beast::detail::base64;\n\n")
    endif()
    
    set(ALL_CONTENT "${ALL_CONTENT}${HEADER_CONTENT}\n")
endforeach()

# For json.h, we need to add the base64 namespace alias
set(JSON_SECTION "// =============================================================================\n// BEGIN: json.h (Main serialization library)\n// =============================================================================\n\nnamespace base64 = boost::beast::detail::base64;\n")

# Build the list of header names for the comment
set(HEADER_LIST "")
foreach(HEADER ${ORDERED_HEADERS})
    get_filename_component(HEADER_NAME ${HEADER} NAME)
    set(HEADER_LIST "${HEADER_LIST}//   - ${HEADER_NAME}\n")
endforeach()

# Build the combined header
set(COMBINED_HEADER "// =============================================================================
// JSON Serialization Library - Single Header Distribution
// =============================================================================
// This file combines all necessary headers for easy integration.
// Simply include this file in your project to use the JSON library.
//
// Generated automatically by CMake from:
${HEADER_LIST}//
// Copyright: See individual headers for copyright information
// =============================================================================

#ifndef JSON_ALL_H_INCLUDED
#define JSON_ALL_H_INCLUDED

// External dependencies (must be available in your project)
#include <boost/json.hpp>
#include <boost/json/static_resource.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/optional.hpp>

// Standard library includes
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
${ALL_CONTENT}
#endif // JSON_ALL_H_INCLUDED
")

# Write the combined header
file(WRITE "${OUTPUT_FILE}" "${COMBINED_HEADER}")

message(STATUS "Generated json_all.h successfully")
