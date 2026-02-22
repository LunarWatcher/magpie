add_executable(tests 
    src/Main.cpp

    src/exception/ErrorHandlerTests.cpp

    src/integration/BasicServerTests.cpp
    src/integration/CatchAllTests.cpp
    src/integration/CompressionTests.cpp
    src/integration/MiddlewareTests.cpp
    src/integration/RouteParamTests.cpp
    src/integration/ServerErrorHandlerTests.cpp

    src/integration/regression/StatusCodeOnSubtreeTests.cpp

    src/routing/RouteCompilationTests.cpp
    src/routing/RouterTests.cpp

    src/transfer/adapters/FixedDataAdapterTests.cpp
)

Include(FetchContent)

set(CATCH_CONFIG_DEFAULT_REPORTER "explicit" CACHE STRING "" FORCE)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.12.0
)


FetchContent_Declare(stc
    GIT_REPOSITORY https://github.com/LunarWatcher/stc
)

FetchContent_MakeAvailable(Catch2 stc)

find_package(cpr REQUIRED)

target_link_libraries(tests Catch2::Catch2 stc::stc stc::testutil )
target_link_libraries(tests magpie cpr::cpr)

target_include_directories(tests PUBLIC src/)

# vim:ft=cmake
