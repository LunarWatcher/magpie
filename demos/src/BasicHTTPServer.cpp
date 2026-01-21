#include "magpie/App.hpp"
#include "magpie/data/CommonData.hpp"

struct Sessions { };

struct Context : public magpie::data::CommonData {
    Sessions sess;
};

int main() {
    std::shared_ptr<Context> ctx = std::make_shared<Context>();
    magpie::App<Context> app {
        magpie::AppConfig {
            .port = 8080
        }
    };

    app.route<"/">([](Context*) {
        std::cout << "You have sunk my battleship" << std::endl;
    });

    app.route<"/{string}">([](Context*, const std::string_view& v) {
        std::cout << "URL component: " << v << std::endl;
    });
    app.route<"/test/{string}">([](Context*, const std::string_view& v) {
        std::cout << "Subroute: " << v << std::endl;
    });

    app.run(true);
}
