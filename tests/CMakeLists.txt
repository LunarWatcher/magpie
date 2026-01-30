add_executable(tests 
    src/integration/BasicServerTests.cpp
    src/integration/RouteParamTests.cpp

    src/routing/RouteCompilationTests.cpp
    src/routing/RouterTests.cpp
)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(Catch2)

find_package(cpr REQUIRED)

target_link_libraries(tests Catch2::Catch2WithMain)
target_link_libraries(tests magpie cpr::cpr)

target_include_directories(tests PUBLIC src/)

# vim:ft=cmake
