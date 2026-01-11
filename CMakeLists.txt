cmake_minimum_required(VERSION 3.27)
project(magpie)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message (STATUS "magpie: Running standalone")
    set(MAGPIE_STANDALONE ON)

    if (NOT WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    endif()

    option(MAGPIE_ENABLE_DEMOS "Whether or not to build demos" ON)
    option(MAGPIE_BUILD_TESTS "Whether or not to build tests" ON)
    option(MAGPIE_DOCS "Whether or not to build doxygen docs" OFF)
    option(MAGPIE_SANITISE "Whether or not to -fsanitise=undefined" OFF)
else()
    message (STATUS "alui: Running as a submodule")
    set(MAGPIE_STANDALONE OFF)

    option(MAGPIE_ENABLE_DEMOS "Whether or not to build demos" OFF)
    option(MAGPIE_BUILD_TESTS "Whether or not to build tests" OFF)
    option(MAGPIE_DOCS "Whether or not to build doxygen docs" OFF)
    option(MAGPIE_SANITISE "Whether or not to -fsanitise=undefined" OFF)
endif()

message(STATUS "Magpie build configuration:")
message(STATUS "\t- Demos: ${MAGPIE_ENABLE_DEMOS}")
message(STATUS "\t- Tests: ${MAGPIE_BUILD_TESTS}")
message(STATUS "\t- Docs: ${MAGPIE_DOCS}")
message(STATUS "\t- Sanitising: ${MAGPIE_SANITISE}")

# Used for crypto, and maybe HTTPS eventually:tm:
find_package(OpenSSL REQUIRED)
find_package(nghttp2 REQUIRED)
find_package(asio REQUIRED)

add_subdirectory(src)

add_custom_target(run
    COMMAND magpie
    DEPENDS magpie
    COMMENT "Run magpie")
if (MAGPIE_BUILD_TESTS)
    add_subdirectory(tests EXCLUDE_FROM_ALL)
    add_custom_target(test
        COMMAND tests
        DEPENDS tests
        COMMENT "Test magpie")
endif()
if (MAGPIE_ENABLE_DEMOS)
    add_subdirectory(demos)
endif()
# vim:ft=cmake
