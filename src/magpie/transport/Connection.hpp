#pragma once

#include "magpie/application/Http2Adapter.hpp"
#include <asio.hpp>
#include <iostream>
#include <memory>

namespace magpie::transport {

class Connection : public std::enable_shared_from_this<Connection> {
private:
    std::array<char, 4096> recv;
    // TODO: make the protocol dynamic
    application::Http2Adapter adapter;

    void doRead() {
        auto self = shared_from_this();
        socket.async_read_some(
            asio::buffer(recv),
            [self](const asio::error_code& ec, size_t bytes) {
                if (!ec && bytes > 0) {
                    self->adapter.parse(
                        self->recv,
                        bytes
                    );

                    self->doRead();
                }
            }
        );
    }

    void doBulkWrite(
        const std::string& data
    ) {
        auto self = shared_from_this();
        asio::async_write(
            self->socket,
            asio::buffer(data),
            [](const asio::error_code& err, size_t) {
                if (err) {
                    // TODO: how do I want to do error handling on a level this low? 
                    // Logging does make sense, but it probably makes more sense to integrate with the consuming app's
                    // default logging system (or just std::cout if none is provided)
                    std::cout << err.message() << "\n";
                }
            }
        );
    }
public:
    asio::ip::tcp::socket socket;

    Connection(asio::io_context& ctx)
        : adapter(this), socket(ctx) {}

    void start() {
        doRead();
    }
};

}
