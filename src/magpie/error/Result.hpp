#pragma once

#include <stdexcept>
#include <variant>

namespace magpie {

template <typename T, typename ERR>
struct Result {
private:
    using InternalType = std::variant<T, ERR>;
    const InternalType held;

    Result(InternalType&& value) 
        : held(std::move(value)) {}

public:
    const T* operator->() const {
        if (held.index() != 0) {
            throw std::runtime_error("Illegal access into error result");
        }

        return &(std::get<T>(held));
    }

    bool isOk() const { return held.index() == 0; }
    bool isError() const { return held.index() == 1; }

    const T& unwrap() { return std::get<0>(held); }
    const ERR& error() { return std::get<1>(held); }

    static Result<T, ERR> ok(T&& value) {
        return Result(InternalType(std::in_place_index<0>, std::move(value)));
    }
    static Result<T, ERR> err(ERR&& value) {
        return Result(InternalType(std::in_place_index<1>, std::move(value)));
    }
};

}
