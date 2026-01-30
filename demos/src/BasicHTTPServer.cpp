#include "magpie/App.hpp"
#include "magpie/config/SSLConfig.hpp"
#include "magpie/data/CommonData.hpp"
#include "magpie/transfer/Response.hpp"

struct Sessions { };

struct Context : public magpie::data::CommonData {
    Sessions sess;
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

    app.route<"/", magpie::Method::Get>([](Context*, magpie::Request&, magpie::Response& res) {
        res = magpie::Response(
            magpie::Status::OK, "Good girl :3"
        );
    });

    app.route<"/{string}", magpie::Method::Get>([](Context*, magpie::Request&, auto& res, const std::string_view& v) {
        res = magpie::Response(
            magpie::Status::IM_A_TEAPOT, std::format(
                "Where is your god now, {}?", v
            )
        );
    });
    app.route<"/{int}", magpie::Method::Get>([](Context*, magpie::Request&, auto& res, int64_t v) {
        if (v == 69) {
            res.body = "Nice";
        } else {
            res.body = std::move(
                std::format("Your lucky number is {}", v)
            );
        }
    });
    app.run();
}
