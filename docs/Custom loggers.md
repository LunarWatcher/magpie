# Custom loggers

Magpie's built-in logger is intentionally minimal, and not particularly performant. It is, however, designed to be easy to replace. This is also why the built-in logger doesn't contain many features.

Here's an example of replacing the default logger with stc's `minilog`[^1]:
```cpp
magpie::logger::config().logger = [](auto level, const auto& msg) {
    switch (level) {
        case magpie::logger::Level::debug:
            minilog::debug("{}", msg);
            break;
        case magpie::logger::Level::info:
            minilog::info("{}", msg);
            break;
        case magpie::logger::Level::warning:
            minilog::warn("{}", msg);
            break;
        case magpie::logger::Level::error:
            minilog::error("{}", msg);
            break;
        case magpie::logger::Level::critical:
            minilog::critical("{}", msg);
            break;
    }
};
```

Magpie's built-in logger provides no fancy facilities; it has no log level control, and assumes the underlying logger deals with all that. This means that if you don't want to log debug messages, debug messages need to be disabled in the underlying logger.

One disadvantage with magpie's logging system is that it cannot currently forward the `std::format` arguments, so an extra format step has to happen. As a consequence, **do not directly forward the message as the first argument to any logger**; though you should be prevented from doing so on account of the string being dynamic, if your logger is able to do dynamic format strings, this is a footgun. This is a very easy way to footgun yourself if there ever is a log4j-style vulnerability in whatever logger you use.

> [!tip]
>
> Although you can use `magpie::logger` to abstract away your logger into a framework-specific thing, the extra format layer means it's recommended to not do this. `magpie::logger` is primarily intended for internal use, i.e. how magpie's log messages are shown.
>
> This also means that, by not using `magpie::logger`, you can use a dedicated logger to directly control specifically magpie's output without affecting the rest of your application, provided your logging library supports this.

[^1]: Minilog is also not particularly performant, likely as slow as magpie's built-in logger, but it is prettier.
