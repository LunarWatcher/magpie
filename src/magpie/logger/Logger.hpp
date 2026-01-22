#pragma once

#include <functional>
#include <iostream>
#include <string_view>
#include <format>

namespace magpie::logger {

enum class Level {
    debug,
    info,
    warning,
    error,
    critical
};

inline void defaultHandler(Level level, const std::string_view& message) {
    switch (level) {
    case Level::debug:
        std::cout << "DEBUG    " << message << std::endl;
        break;
    case Level::info:
        std::cout << "INFO     " << message << std::endl;
        break;
    case Level::warning:
        std::cout << "WARNING  " << message << std::endl;
        break;
    case Level::error:
        std::cout << "ERROR    " << message << std::endl;
        break;
    case Level::critical:
        std::cout << "CRITICAL " << message << std::endl;
        break;
    }
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
    log<Level::debug, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void info(const std::format_string<Args...>& format, Args&&... args) {
    log<Level::info, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void warn(const std::format_string<Args...>& format, Args&&... args) {
    log<Level::warning, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void error(const std::format_string<Args...>& format, Args&&... args) {
    log<Level::error, Args...>(format, std::forward<Args>(args)...);
}

template <class... Args>
inline void critical(const std::format_string<Args...>& format, Args&&... args) {
    log<Level::critical, Args...>(format, std::forward<Args>(args)...);
}

}
