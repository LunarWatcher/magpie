#pragma once

#include "magpie/application/Adapter.hpp"
#include "magpie/transfer/Request.hpp"

namespace magpie::application {

class Http2Adapter : public Adapter {
public:
    void start(
        const std::function<
            const Request&
        >& onRequest
    ) override;
    void stop() override;
};

}
