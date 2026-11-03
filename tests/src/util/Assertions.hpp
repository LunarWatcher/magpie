#pragma once

#define REQUIRE_OK_TEXT(res, expectedText)          \
    INFO(res.error.message);                        \
    REQUIRE(res.status_code == magpie::Status::OK); \
    REQUIRE(res.text == expectedText)
