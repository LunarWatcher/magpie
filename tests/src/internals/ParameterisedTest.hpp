#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <functional>
#include <catch2/catch_test_macros.hpp>

namespace test {

template <typename Expected, typename ...InputArgs>
struct ParameterisedTestData {
    std::tuple<InputArgs...> input;
    Expected output;
    std::string testCaseName;


    template <typename R>
    R forward(const std::function<R(InputArgs...)>& func) const {
        return std::apply(func, input);
    }

    void forward(const std::function<void(InputArgs...)>& func) const {
        std::apply(func, input);
    }
};

template <typename Expected, typename ...InputArgs>
struct ParameterisedTest {
    using TestData = ParameterisedTestData<Expected, InputArgs...>;
    std::vector<TestData> data;
    std::function<void(const TestData&)> executor;

    constexpr ParameterisedTest(
        std::vector<TestData> data,
        std::function<void(const TestData&)> executor
    ): data(data), executor(executor) {
        if (executor == nullptr) {
            throw std::runtime_error("Executor cannot be nullptr");
        }
    }

    void run() {
        for (const auto& entry : data) {
            SECTION(entry.testCaseName) {
                executor(entry);
            }
        }
    }

};

}
