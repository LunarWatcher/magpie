#pragma once

#include "magpie/logger/Logger.hpp"
#include <string>
#include <functional>

namespace magpie::utility {

/**
 * Utility function used for handling errors. Currently only used for error logging, but could be expanded with
 * callbacks in the future.
 *
 * The one place it's currently used (TCPServer) uses this function at a low level where the implementer would not have
 * a chance to handle anything anyway. We're talking primarily low-level TCP errors, or other catastrophic low-level
 * failures where the user simply cannot be notified.
 *
 * Until proper application-level error handling is implemented, this also includes all application-level errors. These
 * will be handled separately, and likely also piped through either a middleware or a special error handler. Not sure
 * how I want to structure that yet.
 */
constexpr void runWithErrorLogging(
    std::function<void()> errorHandled
) {

    try {
        errorHandled();
    } catch (const std::exception& e) {
        logger::error(
            "Uncaught exception: {}", e.what()
        );
    } catch (const std::string& e) {
        logger::error(
            "Uncaught {{str}}exception: {}", e
        );
    } catch (...) {
        // TODO: how does catch2 do this kind of logging? I doubt they have a full cascade of every single
        // type that may be caught in anything ever
        // Might be template fuckery though
        logger::critical(
            "Uncaught exception of unknown type (cannot log)"
        );
    }
}

}
