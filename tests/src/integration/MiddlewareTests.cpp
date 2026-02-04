#include "integration/TestApp.hpp"
#include "magpie/data/CommonData.hpp"
#include "magpie/transfer/Response.hpp"
#include <string>
#include <catch2/catch_test_macros.hpp>

struct MiddlewareTestContext : public magpie::data::CommonData {
    const std::string constant = "trans rights are human rights";
    int var = 0;
};

TEST_CASE("Verify that the context is set as expected") {
    TestApp<MiddlewareTestContext> app;

    app->route<"/", magpie::Method::Get>(
        [](MiddlewareTestContext* ctx, auto&, auto& res) {
            REQUIRE(ctx != nullptr);
            REQUIRE(ctx->app != nullptr);
            REQUIRE(ctx->constant == "trans rights are human rights");

            ctx->var = 69;

            res = magpie::Response(magpie::Status::OK, "Response");
        }
    );

    app.start();

    auto res = app.Get(app.url());

    REQUIRE(res.status_code == 200);
    REQUIRE(res.text == "Response");

    REQUIRE(static_cast<MiddlewareTestContext*>(app->getContext())->var == 69);
}
