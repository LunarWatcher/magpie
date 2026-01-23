#include "Connection.hpp"

#include "magpie/App.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>

namespace magpie::transport {
// SSL* Connection::setupSSL(BaseApp* app) {
//     return app->getConfig().ssl.and_then(
//         [](const auto& ssl) {
//             SSL* sslPtr = SSL_new(ssl.sslCtx);

//             if (!sslPtr) {
//                 // TODO: handle properly
//                 throw std::runtime_error(
//                     std::string {
//                         ERR_error_string(ERR_get_error(), nullptr)
//                     }
//                 );
//             }

//             return std::optional<SSL*>(
//                 sslPtr
//             );
//         }
//     ).value_or(nullptr);
// }

} // namespace magpie::transport
