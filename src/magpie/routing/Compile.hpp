#pragma once

#include "magpie/data/CommonData.hpp"
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace magpie::routing {

struct ConstString;
template <size_t N>
struct FixedString {
    std::array<char, N + 1> data{};
    const size_t size = N;

    constexpr FixedString(const char (&str)[N + 1]) {
        std::copy_n(str, N + 1, std::data(data));
    }
    constexpr explicit FixedString(const ConstString& src);

    constexpr explicit FixedString(const char* begin) {
        std::copy_n(begin, N, std::data(data));
        data.back() = '\0';
    }
    consteval char operator[](size_t idx) const {
        if (idx >= size) {
            throw std::runtime_error("Invalid string index");
        }
        return *(data.data() + idx);
    }
    constexpr ConstString toConstString() const;

    constexpr auto c_str() const -> char const* { return std::data(data); }
};

template <std::size_t N>
FixedString(const char (&str)[N]) -> FixedString<N - 1>;

struct ConstString {
    const char* str;
    size_t size;

    constexpr ConstString() {
        str = nullptr;
        size = 0;
    }
    template <size_t N>
    constexpr ConstString(const char (&str)[N]): str(str), size(N) {
        static_assert(N >= 1, "Invalid string");
    }
    constexpr ConstString(const ConstString& other): str(other.str), size(other.size) {}

    constexpr explicit ConstString(const char* str, size_t N): str(str), size(N) {
        if (N <= 1) {
            throw std::runtime_error("Invalid string");
        }
    }

    template <size_t N, size_t Offset>
    constexpr FixedString<N> subrangeToFixedString() const {
        if (Offset + N >= size) {
            throw std::runtime_error("offset and length is out of bounds");
        }
        return FixedString<N>(str + Offset);
    }

    constexpr void operator=(const ConstString& rhs) {
        this->str = rhs.str;
        this->size = rhs.size;
    }

    constexpr char operator[](size_t idx) const {
        if (idx >= size) {
            throw std::runtime_error("Invalid string index");
        }
        return *(str + idx);
    }
    constexpr const char* end() const { return str + size; }
};

template <size_t N>
constexpr ConstString FixedString<N>::toConstString() const {
    return ConstString((const char*) std::data(data), N + 1);
}
template <size_t N>
constexpr FixedString<N>::FixedString(const ConstString& src) {
    std::copy_n(src.str, N + 1, std::data(data));
}

template <int offset>
constexpr bool startsWithAtOffset(const ConstString& source, const ConstString& substring) {
    if (substring.size > source.size - offset) { 
        return false;
    }

    for (size_t i = 0; i < substring.size; ++i) {
        if (source[i + offset] != substring[i]) {
            return false;
        }
    }
    return true;
}

constexpr bool startsWithAtOffset(const ConstString& source, const ConstString& substring, size_t offset) {
    if (substring.size > source.size - offset) { 
        return false;
    }

    for (size_t i = 0; i < substring.size; ++i) {
        if (source[i + offset] != substring[i] && substring[i] != '\0') {
            return false;
        }
    }
    return true;
}

template <FixedString source>
constexpr size_t guessParams() {
    size_t result = 0;
    for (size_t i = 0; i < source.size; ++i) {
        if (source[i] == '{') {
            ++result;
        }
    }
    return result;
}

template <FixedString s>
struct TypeInfo : public std::false_type {
    using type = void;
};

template<>
struct TypeInfo<"{int}"> {
    using type = int64_t;
    static constexpr type convert(const std::string_view& v) {
        type out;
        std::from_chars(v.begin(), v.end(), out);
        return out;
    }
};

template<>
struct TypeInfo<"{string}"> {
    using type = std::string_view;
    static constexpr type convert(const std::string_view& v) {
        return v.back() == '/' ? 
            v.substr(0, v.size() - 1)
            : v;
    }
};

constexpr static inline ConstString INT_VALUE("{int}", 6);
constexpr static inline ConstString STRING_VALUE("{string}", 9);

template <FixedString s, size_t matched, size_t params>
constexpr void parse(std::array<ConstString, params>& out, size_t i = 0) {
    if constexpr (matched == params) {
        return;
    } else {
        if (i >= s.size) {
            return;
        } 
        if (s[i] == '{') {
            constexpr auto constStr = s.toConstString();
            if (startsWithAtOffset(constStr, INT_VALUE, i)) {
                out.at(matched) = INT_VALUE;
                parse<s, matched + 1, params>(out, i + 5);
            } else if (startsWithAtOffset(constStr, STRING_VALUE, i)) {
                out.at(matched) = STRING_VALUE;
                parse<s, matched + 1, params>(out, i + 8);
            } else {
                throw "invalid";
            }
        } else {
            parse<s, matched, params>(out, i + 1);
        }
    }
}

template <FixedString s, size_t matched = 0, size_t params = guessParams<s>()>
constexpr std::array<ConstString, params> getParameterTypes() {
    if constexpr (params == 0) {
        return std::array<ConstString, params>{};
    } else {
        std::array<ConstString, params> out;
        parse<s, matched, params>(out);

        return out;
    }
}

template <FixedString s, size_t matched, size_t params>
constexpr void parseForIndices(
    std::array<std::pair<ConstString, size_t>, params>& out,
    size_t slashIndex = 0,
    size_t i = 1
) {
    if constexpr (matched == params) {
        return;
    } else {
        auto newSlashIndex = slashIndex + (s[i] == '/');
        if (i >= s.size) {
            return;
        } 
        if (s[i] == '{') {
            constexpr auto constStr = s.toConstString();
            if (startsWithAtOffset(constStr, INT_VALUE, i)) {
                std::get<matched>(out) = {INT_VALUE, slashIndex};
                parseForIndices<s, matched + 1, params>(out, newSlashIndex, i + 5);
            } else if (startsWithAtOffset(constStr, STRING_VALUE, i)) {
                std::get<matched>(out)  = {STRING_VALUE, slashIndex};
                parseForIndices<s, matched + 1, params>(out, newSlashIndex, i + 8);
            } else {
                throw "invalid";
            }
        } else {
            parseForIndices<s, matched, params>(out, newSlashIndex, i + 1);
        }
    }
}

template <FixedString s, size_t Size = guessParams<s>()>
consteval auto getForwardableIndices() {
    std::array<std::pair<ConstString, size_t>, Size> out;
    parseForIndices<s, 0, Size>(out);
    return out;
}

template <FixedString s>
consteval bool isValidPath() {
    for (size_t i = 0; i < s.size; ++i) {
        if (s[i] == '{') {
            if (i == 0 || s[i - 1] != '/') {
                return false;
            }
        } else if (s[i] == '}') {
            if (i != s.size - 1 && s[i + 1] != '/') {
                return false;
            }
        }
    }
    return true;
}

template <
    FixedString s,
    typename ReturnType = void,
    typename ...Extras
>
struct FunctionSignature {
private:
    static_assert(s[0] == '/', "Invalid route: must start with /");
    static_assert(isValidPath<s>(), "Invalid route: each placeholder value must be a separate segment");

    constexpr static auto typeArray = getParameterTypes<s>();

    template <std::size_t... I>
    static auto compileType(std::index_sequence<I...>)
        -> typename std::function<
            ReturnType(
                Extras...,
                typename TypeInfo<
                    FixedString<typeArray[I].size - 1>(typeArray[I])
                >::type...
            )
        >;

public:
    using type = decltype(
        compileType(
            std::make_index_sequence<typeArray.size()>()
        )
    );

};

}
