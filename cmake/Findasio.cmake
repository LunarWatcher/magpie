find_package(Threads REQUIRED)
find_path(ASIO_INCLUDE_DIR asio.hpp)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(asio
    FOUND_VAR ASIO_FOUND
    REQUIRED_VARS ASIO_INCLUDE_DIR
)

if(ASIO_FOUND AND NOT TARGET asio::asio)
    add_library(asio::asio INTERFACE IMPORTED)
    target_include_directories(asio::asio
        INTERFACE
        ${ASIO_INCLUDE_DIR}
        )
    target_compile_definitions(asio::asio
        INTERFACE
        "ASIO_STANDALONE"
    )
    target_link_libraries(asio::asio
        INTERFACE
        Threads::Threads
    )
endif()

mark_as_advanced(ASIO_FOUND ASIO_INCLUDE_DIR)
