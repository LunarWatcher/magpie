add_executable(tests 
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

target_link_libraries(tests Catch2::Catch2WithMain)
target_link_libraries(tests magpie)

target_include_directories(tests PUBLIC src/)

# vim:ft=cmake
