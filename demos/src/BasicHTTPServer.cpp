#include "magpie/App.hpp"
int main() {
    magpie::App app {
        magpie::AppConfig {
            .port = 8080
        }
    };

    app.run(true);
}
