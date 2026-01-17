#pragma once


#include <type_traits>
namespace magpie::data {

class App;
struct CommonData {
    App* app;

    virtual ~CommonData() = default;
};

template <typename T>
concept IsCommonData = requires {
    std::is_base_of_v<data::CommonData, T>;
};

}
