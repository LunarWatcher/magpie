#pragma once

#include "magpie/config/SSLConfig.hpp"
#include <optional>
#include <thread>

namespace magpie {

struct AppConfig {
    uint16_t port = 8080;
    unsigned int concurrency = std::thread::hardware_concurrency();

    std::optional<SSLConfig> ssl = std::nullopt;
};


}
