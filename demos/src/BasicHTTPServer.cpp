#include "magpie/App.hpp"
#include "magpie/config/SSLConfig.hpp"
#include "magpie/data/CommonData.hpp"
#include "magpie/transfer/Response.hpp"
#include "magpie/transfer/CompressedResponse.hpp"
#include "magpie/transfer/StatusCode.hpp"

struct Sessions { };

struct Context : public magpie::data::CommonData {
    Sessions sess;
};

class TestMiddleware : public magpie::Middleware<Context> {
public:
    void onRequest(
        magpie::IMiddlewareProcessor<Context> *proc,
        Context* ctx,
        magpie::Request& req,
        magpie::Response& res
    ) override {
        if (req.headers.contains("x-reject")) {
            res = magpie::Response(
                magpie::Status::BadRequest,
                "Now you're gone"
            );
        } else {
            next(proc, ctx, req, res);
        }
        res.headers["Server"] = "magpie-demo-basic";
    }
};

int main() {
    std::shared_ptr<Context> ctx = std::make_shared<Context>();
    magpie::App<Context> app {
        ctx,
        magpie::AppConfig {
            .port = 8080,
            // Note: fromGeneratedCertificate is for test use only. You should lock this behind a macro or other form of
            // check. Never use this in a production environment.
            .ssl = magpie::SSLConfig::fromGeneratedCertificate(),
        },
    };

    app.registerGlobalMiddlewares({
        std::make_shared<TestMiddleware>(),
    });

    app.route<"/", magpie::Method::Get>([](Context*, magpie::Request&, magpie::Response& res) {
        res = magpie::Response(
            magpie::Status::OK, "Good girl :3"
        );
    });

    app.route<"/{string}", magpie::Method::Get>([](Context*, magpie::Request&, auto& res, const std::string_view& v) {
        res = magpie::Response(
            magpie::Status::ImATeapot, std::format(
                "Where is your god now, {}?", v
            )
        );
    });
    app.route<"/{int}", magpie::Method::Get>([](Context*, magpie::Request&, auto& res, int64_t v) {
        if (v == 69) {
            res.setBody("Nice");
        } else {
            res.setBody(
                std::format("Your lucky number is {}", v)
            );
        }
    });
    app.route<"/echo", magpie::Method::Post>([](Context*, magpie::Request& req, auto& res) {
        res = magpie::CompressedResponse(
            req,
            magpie::Status::OK,
            std::move(req.body),
            "text/plain"
        );
    });
    app.run();
}
