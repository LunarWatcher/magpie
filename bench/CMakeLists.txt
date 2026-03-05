add_executable(bench
    src/Main.cpp
)
target_link_libraries(bench PUBLIC magpie)
target_include_directories(bench PUBLIC src/)
