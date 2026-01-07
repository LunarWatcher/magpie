cmake_minimum_required(VERSION 3.27)
project(magpie)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(tests EXCLUDE_FROM_ALL)

add_custom_target(run
    COMMAND magpie
    DEPENDS magpie
    COMMENT "Run magpie")
add_custom_target(test
    COMMAND tests
    DEPENDS tests
    COMMENT "Test magpie")
# vim:ft=cmake
