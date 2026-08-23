#include "raven/conn/CommonDefs.hpp"
#include <catch2/catch_test_macros.hpp>

#include <magpie/application/http11/Http11Parser.hpp>

namespace {

struct Data {
    raven::Buffer buff;
    size_t messageLength;
};

Data initBuffer(const std::string& msg) {
    raven::Buffer buff;
    std::copy(msg.begin(), msg.end(), buff.data());
    return Data {
        buff,
        msg.size()
    };
}

TEST_CASE("Verify HTTP/1.1 parser state transitions", "[http11]") {
    magpie::http11::Http11State parser;
    SECTION("Valid init line should be handled") {
        auto [buff, msgLen] = initBuffer("GET / HTTP/1.1\r\n");
        REQUIRE(
            parser.read(buff, msgLen)
            ==
            magpie::http11::ServerAction::ContinueRead
        );
        REQUIRE(parser.state == magpie::http11::Http11ParserState::ReadHeader);

        REQUIRE(parser.req == nullptr);
    }

    SECTION("Headers with no body should be parsed") {
        auto [buff, msgLen] = initBuffer(
            "GET / HTTP/1.1\r\n"
            "Cookie: a=b\r\n"
            "\r\n"
        );
        REQUIRE(
            parser.read(buff, msgLen)
            ==
            magpie::http11::ServerAction::SetResponse
        );
        REQUIRE(parser.state == magpie::http11::Http11ParserState::WriteHeaders);

        REQUIRE(parser.req != nullptr);
        REQUIRE(parser.res == nullptr);
    }

    SECTION("Request with body should be parsed") {
        auto [buff, msgLen] = initBuffer(
            "GET / HTTP/1.1\r\n"
            "Content-Length: 12\r\n"
            "\r\n"
            "good girl :3"
        );
        REQUIRE(
            parser.read(buff, msgLen)
            ==
            magpie::http11::ServerAction::SetResponse
        );
        REQUIRE(parser.state == magpie::http11::Http11ParserState::WriteHeaders);

        REQUIRE(parser.req != nullptr);
        REQUIRE(parser.res == nullptr);

        REQUIRE(parser.req->body == "good girl :3");
    }
}

TEST_CASE("The HTTP/1.1 parser needs to support arbitrary splits from read()", "[http11]") {
    magpie::http11::Http11State parser;

    SECTION("Sectioned by type") {
        {
            auto [buff, msgLen] = initBuffer(
                "GET / HTTP/1.1\r\n"
                    "Host: example.com\r\n"
            );
            REQUIRE(
                parser.read(buff, msgLen)
                ==
                magpie::http11::ServerAction::ContinueRead
            );
        }
        REQUIRE(parser.state == magpie::http11::Http11ParserState::ReadHeader);
        REQUIRE(parser.req == nullptr); // This won't be initialized until after we get the header block
        REQUIRE(parser.res == nullptr);

        SECTION("With body") {
            {
                auto [buff, msgLen] = initBuffer(
                    "Content-Length: 12\r\n"
                    "\r\n"
                );
                REQUIRE(
                    parser.read(buff, msgLen)
                    ==
                    magpie::http11::ServerAction::ContinueRead
                );
            }
            REQUIRE(parser.state == magpie::http11::Http11ParserState::ReadBody);
            {
                auto [buff, msgLen] = initBuffer(
                    "good girl :3"
                );
                REQUIRE(
                    parser.read(buff, msgLen)
                    ==
                    magpie::http11::ServerAction::SetResponse
                );
            }
            REQUIRE(parser.state == magpie::http11::Http11ParserState::WriteHeaders);
            REQUIRE(parser.req->body == "good girl :3");
            REQUIRE(parser.req != nullptr); // This won't be initialized until after we get the header block
        }
        SECTION("Without body") {
            {
                auto [buff, msgLen] = initBuffer(
                    "Host: example.com\r\n"
                    "\r\n"
                );
                REQUIRE(
                    parser.read(buff, msgLen)
                    ==
                    magpie::http11::ServerAction::SetResponse
                );
            }
            REQUIRE(parser.req != nullptr); // This won't be initialized until after we get the header block
            REQUIRE(parser.state == magpie::http11::Http11ParserState::WriteHeaders);
        }


    }
}

}
