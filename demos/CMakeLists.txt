add_executable(
    basic-http-server

    src/BasicHTTPServer.cpp
)
add_executable(basic-http1.1-server src/BasicHTTP11Server.cpp)

target_link_libraries(basic-http-server PRIVATE magpie)
target_link_libraries(basic-http1.1-server PRIVATE magpie)
