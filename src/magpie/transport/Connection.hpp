#pragma once

#include <asio.hpp>
#include <iostream>

namespace magpie::transport {

class Connection {
public:
    asio::ip::tcp::socket socket;

  Connection(asio::io_context& ctx)
    : socket(ctx) {}

  void start() {
    asio::async_write(
        socket,
        asio::buffer("Good girl\n"),
        [](const asio::error_code& err, size_t) {
            if (err) { 
                std::cout << err.message() << "\n";
            }
        }
    );
  }
};

}
