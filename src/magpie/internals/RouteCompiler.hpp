#pragma once

#include <string_view>

namespace magpie::router::compiler {


struct Route {
    template <typename Func>
    void operator()(Func func) {

    }
};

namespace _detail {

constexpr std::string_view parse() {
    std::string_view x;
    return "";
}

}

}
