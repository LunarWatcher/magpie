#include "magpie/transfer/adapters/FixedDataAdapter.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Test FixedAdapter offsets") {
    std::string sourceData = "Good girl :3";
    std::string copiedSourceData = sourceData;
    size_t sourceLength = sourceData.size();
    magpie::FixedDataAdapter adapter{
        std::move(sourceData)
    };

    std::string reconstructed;

    std::array<uint8_t, 2> buffer;
    uint32_t flags = 0;

    REQUIRE(
        adapter.getChunk(2, (uint8_t*) buffer.data(), &flags) == 2
    );
    reconstructed += std::string{(char*) buffer.data(), 2};
    REQUIRE(reconstructed == "Go");
    REQUIRE(adapter.getReadOffset() == 2);

    REQUIRE(
        adapter.getChunk(2, (uint8_t*) buffer.data(), &flags) == 2
    );
    reconstructed += std::string{(char*) buffer.data(), 2};
    REQUIRE(reconstructed == "Good");
    REQUIRE(adapter.getReadOffset() == 4);

    while (flags == 0) {
        auto len = adapter.getChunk(2, (uint8_t*) buffer.data(), &flags);
        REQUIRE(len == 2);
        reconstructed += std::string{ (char*) buffer.data(), len };
    }
    // We don't shift the readOffset if EOF. That's handled by the EOF flag being passed to and handled by nghttp2, or
    // in the future, other adapter implementations.
    // The adapter implementations being this tied to the HTTP protocol is a bit of an issue though
    REQUIRE(adapter.getReadOffset() == sourceLength - 2);
    REQUIRE(reconstructed.size() == sourceLength);
    REQUIRE(reconstructed == copiedSourceData);

}
