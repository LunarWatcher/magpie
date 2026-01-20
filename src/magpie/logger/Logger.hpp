#pragma once

#include <functional>
#include <iostream>
#include <string_view>
#include <format>

namespace magpie::logger {

enum class Level {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

inline void defaultHandler(Level, const std::string_view& message) {
    std::cout << message << std::endl;
}

struct LoggerConfig {
    std::function<void(logger::Level, const std::string_view&)> logger = logger::defaultHandler;
};

inline LoggerConfig& config() {
    static LoggerConfig conf;
    return conf;
}

template <Level level, class... Args>
inline void log(const std::format_string<Args...>& fmt, Args&&... args) {
    // TODO: do I add a prefix?
    config().logger(
        level, std::format(fmt, std::forward<Args>(args)...)
    );
}

template <class... Args>
inline void debug(const std::format_string<Args...>& format, Args&&... args) {
    log<Level::DEBUG, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void info(const std::format_string<Args...>& format, Args&&... args) {
    log<Level::INFO, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void warn(const std::format_string<Args...>& format, Args&&... args) {
    log<Level::WARNING, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void error(const std::format_string<Args...>& format, Args&&... args) {
    log<Level::ERROR, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void critical(const std::format_string<Args...>& format, Args&&... args) {
    log<Level::CRITICAL, Args...>(format, std::forward<Args>(args)...);
}

}
