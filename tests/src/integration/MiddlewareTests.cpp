#include "integration/TestApp.hpp"
#include "magpie/data/CommonData.hpp"
#include "magpie/middlewares/Middleware.hpp"
#include "magpie/transfer/Response.hpp"
#include "magpie/transfer/StatusCode.hpp"
#include <string>
#include <catch2/catch_test_macros.hpp>


struct MiddlewareTestContext : public magpie::data::CommonData {
    const std::string constant = "trans rights are human rights";
    int var = 0;
};

class FuckeryMiddleware : public magpie::Middleware<MiddlewareTestContext> {
public:
    void onRequest(
        magpie::IMiddlewareProcessor<MiddlewareTestContext> *proc,
        MiddlewareTestContext* ctx,
        magpie::Request& req,
        magpie::Response& res
    ) override {
        // TODO: headers being forced lower-case probably warrants a wrapper around unordered_map or something
        if (req.headers.contains(std::string("x-fuckery"))) {
            res = magpie::Response (
                magpie::Status::Gone,
                "Now you're gone"
            );
        } else {
            res.headers["before"] = "yes";
            next(proc, ctx, req, res);
            res.headers["after"] = "yes";
        }
    }

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

TEST_CASE("Verify that a single middleware works as expected") {
    auto peekableContext = std::make_shared<MiddlewareTestContext>();
    TestApp<MiddlewareTestContext> app(
        peekableContext
    );

    app->registerGlobalMiddlewares({
        std::make_shared<FuckeryMiddleware>()
    });

    // same route and validations as Verify that the context is set as expected
    // We want to revalidate with a middleware interfering
    app->route<"/", magpie::Method::Get>(
        [](MiddlewareTestContext* ctx, auto&, auto& res) {
            REQUIRE(ctx != nullptr);
            REQUIRE(ctx->app != nullptr);
            REQUIRE(ctx->constant == "trans rights are human rights");

            ctx->var = 69;
            res = magpie::Response(magpie::Status::OK, "Response");
        }
    );
    app->route<"/non-move-init", magpie::Method::Get>(
        [](MiddlewareTestContext* ctx, auto&, auto& res) {
            REQUIRE(ctx != nullptr);
            REQUIRE(ctx->app != nullptr);
            REQUIRE(ctx->constant == "trans rights are human rights");

            ctx->var = 69;
            res.code = &magpie::Status::ImATeapot;
            res.setBody("Would you care for a spot of tea?");
        }
    );
    app.start();
    REQUIRE(peekableContext->var == 0);

    SECTION("Plain request is not interfered with") {
        auto res = app.Get(app.url());

        INFO(res.error.message);
        INFO(res.text);
        REQUIRE(res.status_code == 200);
        REQUIRE(res.text == "Response");

        REQUIRE(static_cast<MiddlewareTestContext*>(app->getContext())->var == 69);
        {
            INFO("Before the route invocation, header integrity is not guaranteed.");
            REQUIRE_FALSE(res.header.contains("before"));
        }
        {
            INFO("After route invocation, headers should persist");
            REQUIRE(res.header.at("after") == "yes");
        }
    }

    SECTION("Plain request on a non-move assignment endpoint does include the before header") {
        auto res = app.Get(app.url("/non-move-init"));

        INFO(res.error.message);
        INFO(res.text);
        REQUIRE(res.status_code == magpie::Status::ImATeapot);
        REQUIRE(res.text == "Would you care for a spot of tea?");

        REQUIRE(static_cast<MiddlewareTestContext*>(app->getContext())->var == 69);
        REQUIRE(res.header.contains("before"));
        REQUIRE(res.header.at("after") == "yes");
    }

    SECTION("A non-forwarding middleware does not pass through") {
        auto res = app.Get(
            app.url(),
            cpr::Header {
                { "X-Fuckery", "owo" },
            }
        );

        REQUIRE(res.status_code == magpie::Status::Gone);
        REQUIRE(res.text == "Now you're gone");

        REQUIRE(peekableContext->var == 0);
    }
}
