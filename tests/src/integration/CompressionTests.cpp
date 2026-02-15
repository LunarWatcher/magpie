#include "catch2/catch_test_macros.hpp"
#include "integration/TestApp.hpp"
#include "magpie/transfer/CompressedResponse.hpp"
#include "magpie/transfer/StatusCode.hpp"

TEST_CASE("Test compression") {
    TestApp app;

    app->route<"/", magpie::Method::Get>([](auto*, auto& req, magpie::Response& res) {
        res = magpie::CompressedResponse(
            req,
            magpie::Status::OK,
            "Rawr x3 nuzzles pounces on you"
        );
    });
    app.start();
    cpr::Response res;
    
    // TODO: would prefer also having a test for accept-encoding missing entirely, but cpr sets a default encoding, so
    // this is just ignored

    SECTION("Requesting the identity compression should not result in compression") {
        res = app.Get(
            app.url(),
            cpr::Header {
                {"Accept-Encoding", "identity"},
            }
        );
        REQUIRE(res.header.contains("content-encoding"));
        REQUIRE(res.header["content-encoding"] == "identity");
    }

    SECTION("Requesting gzip should result in gzip") {
        res = app.Get(
            app.url(),
            cpr::Header {
                {"Accept-Encoding", "gzip"},
            }
        );
        REQUIRE(res.header.contains("content-encoding"));
        REQUIRE(res.header["content-encoding"] == "gzip");
    }
    SECTION("Requesting deflate should result in deflate") {
        res = app.Get(
            app.url(),
            cpr::Header {
                {"Accept-Encoding", "deflate"},
            }
        );
        REQUIRE(res.header.contains("content-encoding"));
        REQUIRE(res.header["content-encoding"] == "deflate");
    }

    REQUIRE(res.text == "Rawr x3 nuzzles pounces on you");
}
