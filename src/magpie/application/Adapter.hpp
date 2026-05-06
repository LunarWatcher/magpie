#pragma once

#include "raven/conn/CommonDefs.hpp"
#include "raven/conn/Connection.hpp"
#include <cstddef>

namespace magpie::application {

class Adapter : public raven::ConnUserData {
public:
    virtual ~Adapter() = default;

    virtual bool parse(
        const raven::Buffer& buff,
        std::size_t readBytes
    ) = 0;
    virtual bool onWriteReady(
        raven::Connection* conn,
        raven::Buffer&
    ) = 0;
};

}
