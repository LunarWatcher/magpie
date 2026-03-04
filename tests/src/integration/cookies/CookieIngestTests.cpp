#include "integration/TestApp.hpp"
#include "magpie/application/Methods.hpp"
#include "magpie/application/formats/Cookie.hpp"
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cpr/ssl_options.h>

TEST_CASE("Verify that cookies are received correctly", "[Cookies]") {
    TestApp app;
    app->route<"/", magpie::Method::Get>([](auto*, magpie::Request& req, magpie::Response& res) {
        auto cookieResult = req.parseCookies();

        if (cookieResult.isError()) {
            if (cookieResult.error() == magpie::CookieParseError::NoCookies) {
                res = magpie::Response(
                    magpie::Status::NotFound,
                    "no cookies? :'("
                );
                return;
            } else {
                throw std::runtime_error("Unexpected parsing error happened");
            }
        }
        std::stringstream out;
        for (auto& cookie : cookieResult.unwrap()) {
            out << cookie.getName() << "<=>" << cookie.getValue() << "\n";
        }

        res = magpie::Response(
            magpie::Status::OK,
            out.str()
        );
    });
    app.start();

    SECTION("No cookies is a 404") {
        auto res = app.Get(app.url());
        REQUIRE(res.status_code == 404);
    }

    SECTION("One cookie is correct") {
        auto res = app.Get(
            app.url(),
            cpr::Header{
                {"Cookie", "trans_rights_are_human_rights=true"}
            }
        );

        REQUIRE(res.status_code == magpie::Status::OK);
        REQUIRE(res.text == "trans_rights_are_human_rights<=>true\n");
    }

    SECTION("Three cookies is correct") {
        // TODO: Why does cpr::Cookies not work? It causes an internal error (status_code = 0, error.code = 54 (cert
        // error)), which makes no fucking sense. 
        // The cert is fine - yes, it's self-signed, but the root class sets verify=0. if it whines here, it should fail
        // in every other test too.
        auto res = app.Get(
            app.url(),
            cpr::Header{
                {"Cookie", "ai=fuck AI with a cactus; cookies=:pleading_face:; foxes=all"}
            }
        );

        INFO(res.text);
        INFO(static_cast<int>(res.error.code));
        INFO(res.error.message);
        REQUIRE(res.status_code == magpie::Status::OK);
        REQUIRE(res.text == "ai<=>fuck AI with a cactus\n"
            "cookies<=>:pleading_face:\n"
            "foxes<=>all\n");
    }
}
