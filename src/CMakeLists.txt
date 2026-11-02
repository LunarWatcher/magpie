
set(MAGPIE_NGHTTP2_SOURCES
    magpie/application/Http2Adapter.cpp
    magpie/application/http2/Nghttp2Callbacks.cpp
)
add_library(magpie
    magpie/config/AppConfig.cpp

    magpie/application/Common.cpp

    $<$<NOT:$<BOOL:${MAGPIE_DISABLE_HTTP2}>>:${MAGPIE_NGHTTP2_SOURCES}>

    magpie/application/Http11Adapter.cpp
    magpie/application/http11/Http11Parser.cpp

    magpie/application/formats/Cookie.cpp

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

if (MAGPIE_DISABLE_HTTP2)
    target_compile_definitions(magpie PUBLIC -DMAGPIE_DISABLE_HTTP2=ON)
endif()

target_link_libraries(
    magpie
PUBLIC
    ${MAGPIE_NGHTTP2_LIBRARIES}
    raven
    OpenSSL::SSL
    ZLIB::ZLIB
    stc::stc
)

# vim:ft=cmake
