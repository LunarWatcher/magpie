#pragma once

#include <cstddef>
#include <cstdint>

namespace magpie {

/**
 * Base class for data adapters.
 *
 * Data adapters are responsible for mediating between some form of input data, and the output at the HttpAdapter level.
 * The Http2Adapter, for example, is event-driven, and therefore accepts writes in blocks rather than in full chunks of
 * text. Therefore, both streamed and in-memory data being sent must be modified before it actually sets sent to the
 * client. This class defines the standard interface for this process.
 */
class DataAdapter {
public:
    virtual size_t getChunk(
        size_t len,
        uint8_t* buf,
        uint32_t* dataFlags
    ) = 0;

    virtual ~DataAdapter() = default;
};

}
