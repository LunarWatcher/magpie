#pragma once

#include "DataAdapter.hpp"
#include <string>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace magpie {

/**
 * Data adapter for fixed, non-streamed data.
 */
class FixedDataAdapter : public DataAdapter {
private:
    std::string data;

    size_t readOffset = 0;
public:
    FixedDataAdapter(std::string&& data);

    virtual size_t getChunk(
        size_t outLen,
        uint8_t* buf,
        uint32_t* dataFlags
    ) override;
};

}
