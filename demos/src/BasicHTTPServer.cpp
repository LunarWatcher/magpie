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

    });

    app.run(true);
}
