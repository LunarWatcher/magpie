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
    virtual ~DataAdapter() = default;

    virtual size_t getChunk(
        size_t len,
        uint8_t* buf,
        uint32_t* dataFlags
    ) = 0;

    /**
     * Whether or not the adapter is streamed, meaning the length can't be determined ahead of time.
     *
     * This flag has different implications for different protocols. Protocols that don't need to know the length ahead
     * of time can disregard this flag.
     * For protocols that can switch between streamed and fixed length bodies, this can be used to determine if the
     * fixed length or streamed method should be used.
     */
    virtual bool isStreamedAdapter() { return false; }

    /**
     * Returns the length of the content, provided isStreamedAdapter == false.
     * When true, this should always return 0.
     */
    virtual size_t getContentLength() = 0;
};

}
