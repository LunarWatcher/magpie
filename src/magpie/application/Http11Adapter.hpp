#pragma once

#include "magpie/application/Adapter.hpp"
#include "magpie/application/http11/Http11Parser.hpp"
#include "raven/conn/Connection.hpp"

namespace magpie {

class BaseApp;

namespace application {

class Http11Adapter : public Adapter {
private:
    http11::Http11State state;
    BaseApp* app;

    raven::Connection* conn;
public:
    Http11Adapter(
        raven::Connection* conn,
        BaseApp* app
    );
    ~Http11Adapter() = default;

    virtual bool parse(
        const raven::Buffer& buff,
        std::size_t readBytes
    ) override;

    virtual bool onWriteReady(
        raven::Connection* conn,
        raven::Buffer& buff
    ) override;
};

}

}
