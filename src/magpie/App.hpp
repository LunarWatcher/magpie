#pragma once

#include "magpie/transport/TCPServer.hpp"
namespace magpie {

struct AppConfig {
    unsigned short port = 8080;
    unsigned int concurrency = std::thread::hardware_concurrency();
};

class App {
private:
    transport::TCPServer serv;

public:
    App(
        const AppConfig& conf = {}
    );
    ~App();

    void run(
        bool blocking = true
    );

};

}
