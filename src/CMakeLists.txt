add_library(magpie
    magpie/application/Http2Adapter.cpp

    magpie/transport/TCPServer.cpp
)
target_include_directories(
    magpie 
PUBLIC 
    ./
)

target_include_directories(
    magpie
PUBLIC
    ${LIBNGHTTP2_INCLUDE_DIR}
)

target_link_libraries(
    magpie
PUBLIC
    ${LIBNGHTTP2_LIBRARY} # TODO: add_library in Findnghttp2 (it should be compatible with conan though)
    asio::asio
)
# vim:ft=cmake
