#pragma once


#include <type_traits>
namespace magpie { class BaseApp; }
namespace magpie::data {

struct CommonData {
    BaseApp* app;

    virtual ~CommonData() = default;
};

template <typename T>
concept IsCommonData = requires {
    std::is_base_of_v<data::CommonData, T>;
};

}
