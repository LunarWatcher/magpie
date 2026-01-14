#pragma once

#include <array>
#include <cstdint>

namespace magpie::application {

class Adapter {
public:
    virtual ~Adapter() = default;

    virtual void parse(
        const std::array<char, 4096>& buff,
        std::size_t readBytes
    ) = 0;
};

}
