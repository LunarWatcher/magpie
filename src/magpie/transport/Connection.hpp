#pragma once

#include <asio.hpp>
#include <iostream>
#include <memory>

namespace magpie::transport {

class Connection : public std::enable_shared_from_this<Connection> {
private:
    std::array<char, 4096> recv;

    void doRead() {
        auto self = shared_from_this();
        socket.async_read_some(
            asio::buffer(recv),
            [self](const asio::error_code& ec, size_t bytes) {
                if (!ec) {
                    // std::cout << std::string{self->recv.data(), bytes} << std::endl;
                    asio::async_write(
                        self->socket,
                        asio::buffer(std::string{self->recv.data(), bytes}),
                        [](const asio::error_code& err, size_t) {
                            if (err) { 
                                std::cout << err.message() << "\n";
                            }
                        }
                    );
                }
            }
        );
    }
public:
    asio::ip::tcp::socket socket;

    Connection(asio::io_context& ctx)
        : socket(ctx) {}

    void start() {
        doRead();
    }
};

}
