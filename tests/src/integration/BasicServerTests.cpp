#include "magpie/App.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Test plain routing", "[integration]") {
    magpie::App app;

    app.route<"/">([](auto*) {

    });
}
