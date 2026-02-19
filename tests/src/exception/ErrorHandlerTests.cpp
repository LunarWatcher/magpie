#include "magpie/transfer/Response.hpp"
#include "magpie/transfer/StatusCode.hpp"
#include "magpie/utility/ErrorHandler.hpp"
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>

TEST_CASE("Verify `runWithErrorLogging`'s test catching") {
    magpie::Response res;

    SECTION("runtime_error") {
        REQUIRE_NOTHROW(
            magpie::utility::runWithErrorLogging([]() {
                throw std::runtime_error("uwu");
            }, &res)
        );
    }

    SECTION("Int") {
        REQUIRE_NOTHROW(
            magpie::utility::runWithErrorLogging([]() {
                throw 69;
            }, &res)
        );
    }

    SECTION("String") {
        REQUIRE_NOTHROW(
            magpie::utility::runWithErrorLogging([]() {
                throw std::string("Hewwo");
            }, &res)
        );
    }

    INFO(res.code->statusCode);
    REQUIRE((*res.code) == magpie::Status::InternalServerError);
}
