add_library(magpie
    magpie/application/Http2Adapter.cpp
    magpie/application/http2/Nghttp2Callbacks.cpp

    magpie/application/formats/Cookie.cpp

    magpie/config/SSLConfig.cpp

    magpie/transfer/Request.cpp
    magpie/transfer/Response.cpp
    magpie/transfer/CompressedResponse.cpp

    magpie/transfer/adapters/CompressionDataAdapter.cpp
    magpie/transfer/adapters/FixedDataAdapter.cpp

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
    zlib-ng::zlib-ng
    stc::stc
)

# vim:ft=cmake
