#pragma once

#include "magpie/transfer/Request.hpp"
#include <functional>
namespace magpie::application {

class Adapter {
public:
    virtual ~Adapter() = default;

    virtual void start(
        const std::function<
            const Request&
        >& onRequest
    ) = 0;
    virtual void stop() = 0;
};

}
