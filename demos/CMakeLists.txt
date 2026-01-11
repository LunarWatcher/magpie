add_executable(
    basic-http-server

    src/BasicHTTPServer.cpp
)

target_link_libraries(basic-http-server PRIVATE magpie)
