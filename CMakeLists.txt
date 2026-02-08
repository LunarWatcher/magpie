cmake_minimum_required(VERSION 3.27)

if (ENABLE_CONAN)
    include(FetchContent)
    FetchContent_Declare(conan
        GIT_REPOSITORY https://github.com/conan-io/cmake-conan
        GIT_TAG develop2
    )
    FetchContent_MakeAvailable(conan)
    set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES ${conan_SOURCE_DIR}/conan_provider.cmake)
endif()

project(magpie)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (NOT ENABLE_CONAN)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
endif()


if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message (STATUS "magpie: Running standalone")
    set(MAGPIE_STANDALONE ON)

    if (NOT WIN32)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    endif()

    option(MAGPIE_ENABLE_DEMOS "Whether or not to build demos" ON)
    option(MAGPIE_BUILD_TESTS "Whether or not to build tests" ON)
else()
    message (STATUS "alui: Running as a submodule")
    set(MAGPIE_STANDALONE OFF)

    option(MAGPIE_ENABLE_DEMOS "Whether or not to build demos" OFF)
    option(MAGPIE_BUILD_TESTS "Whether or not to build tests" OFF)
endif()

option(MAGPIE_COVERAGE "Whether or not to build with test coverage" OFF)
option(MAGPIE_DOCS "Whether or not to build doxygen docs" OFF)
option(MAGPIE_SANITISE "Whether or not to -fsanitise=undefined. No effect on Windows" OFF)

message(STATUS "Magpie build configuration:")
message(STATUS "\t- Demos: ${MAGPIE_ENABLE_DEMOS}")
message(STATUS "\t- Tests: ${MAGPIE_BUILD_TESTS}")
message(STATUS "\t- Docs: ${MAGPIE_DOCS}")
message(STATUS "\t- Sanitising: ${MAGPIE_SANITISE}")
message(STATUS "\t- Coverage: ${MAGPIE_COVERAGE}")

# Used for crypto, and maybe HTTPS eventually:tm:
find_package(OpenSSL REQUIRED)
find_package(libnghttp2 REQUIRED)
find_package(asio REQUIRED)

add_definitions(-DASIO_NO_DEPRECATED=true -DNGHTTP2_NO_SSIZE_T=true)

# TODO: not sure how useful this actually is. Looks like the tooling struggles with multithreaded tests
if (MAGPIE_COVERAGE)
    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" CMAKE_CXX_COMPILER_VERSION_REGEX_MATCH  ${CMAKE_CXX_COMPILER_VERSION})
    set(COMPILER_MAJOR ${CMAKE_MATCH_1})
    find_program(GCOV NAMES gcov-${COMPILER_MAJOR} gcov REQUIRED)
    find_program(LCOV lcov REQUIRED)
    find_program(GENHTML genhtml REQUIRED)

    add_compile_options(-fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
    add_custom_target(gen-coverage
        COMMAND ${LCOV} --capture --output coverage.info --include "${CMAKE_SOURCE_DIR}/src/\\*" --exclude "${CMAKE_BINARY_DIR}/_deps/\\*" --directory . --gcov-tool ${GCOV} --ignore-errors inconsistent,gcov
        COMMAND ${GENHTML} coverage.info --output coverage-html --ignore-errors inconsistent
    )
endif()

if (NOT WIN32 AND MAGPIE_SANITISE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif()

add_subdirectory(src)

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

if (MAGPIE_DOCS)
    find_package(Doxygen MODULE REQUIRED)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN NO)
    set(DOXYGEN_EXCLUDE_SYMBOLS "std::*")
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "README.md")
    set(DOXYGEN_RECURSIVE YES)
    list(APPEND DOXYGEN_EXCLUDE_PATTERNS "*/build/*" "*/_deps/*")

    doxygen_add_docs(
        doxygen
        ${PROJECT_SOURCE_DIR}
        COMMENT "Generate documentation"
    )
endif()
# vim:ft=cmake
