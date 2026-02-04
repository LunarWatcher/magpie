#include <catch2/catch_session.hpp>
// Not actually unused
#include "stc/test/catch2/ExplicitStreamTestReporter.hpp"

int main(int argc, const char* argv[]) {
    auto sess = Catch::Session();
    return sess.run(argc, argv);
}
