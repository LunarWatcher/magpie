#include "magpie/application/formats/Cookie.hpp"
#include "magpie/error/Result.hpp"
#include "Request.hpp"
#include <vector>
#include <sstream>

namespace magpie {

Result<std::vector<Cookie>, CookieParseError> Request::parseCookies() const {
    using ResultType = Result<std::vector<Cookie>, CookieParseError>;
    constexpr auto STATE_KEY = 0;
    constexpr auto STATE_VALUE = 1;

    auto headerIt = this->headers.find("cookie");
    if (headerIt == this->headers.end()) {
        return ResultType::err(CookieParseError::NoCookies);
    }

    auto& headerValue = headerIt->second;

    if (headerValue.size() == 0) {
        return ResultType::err(CookieParseError::NoCookies);
    }
    std::stringstream accKey, accValue;
    int state = STATE_KEY;

    std::vector<Cookie> out;
    for (size_t i = 0; i < headerValue.size(); ++i) {
        auto ch = headerValue.at(i);
        if (ch == ';') {
            if (state != STATE_VALUE) {
                return ResultType::err(CookieParseError::MissingKeyValuePair);
            }
            state = STATE_KEY;

            std::string key = accKey.str();
            std::string value = accValue.str();
            out.push_back(Cookie(
                std::move(key), std::move(value)
            ));

            accKey.str("");
            accKey.clear();
            accValue.str("");
            accValue.clear();
            if (i != headerValue.size() - 1 && headerValue.at(i + 1) == ' ') {
                ++i;
            }
        } else if (ch == '=' && state == STATE_KEY) {
            state = STATE_VALUE;
        } else if (state == STATE_KEY) {
            accKey << ch;
        } else if (state == STATE_VALUE) {
            accValue << ch;
        }
    }
    std::string key = accKey.str();
    if (!key.empty()) {
        if (state != STATE_VALUE) {
            return ResultType::err(CookieParseError::MissingKeyValuePair);
        }
        std::string value = accValue.str();
        out.push_back(Cookie(
            std::move(key), std::move(value)
        ));
    }
    return ResultType::ok(std::move(out));
}

}
