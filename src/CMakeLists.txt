add_library(magpie
    magpie/application/Http2Adapter.cpp

    magpie/config/SSLConfig.cpp

    magpie/transport/TCPServer.cpp
)
target_include_directories(
    magpie 
PUBLIC 
    ./
)

target_link_libraries(
    magpie
PUBLIC
    libnghttp2::libnghttp2
    asio::asio
    OpenSSL::SSL
)
# vim:ft=cmake
