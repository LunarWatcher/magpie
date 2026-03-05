#pragma once

#include <array>
#include <cstddef>

namespace magpie::application {

using ReadBuffer = std::array<char, 16'384>;
class Adapter {
public:
    virtual ~Adapter() = default;

    virtual bool parse(
        const ReadBuffer& buff,
        std::size_t readBytes
    ) = 0;
};

}
