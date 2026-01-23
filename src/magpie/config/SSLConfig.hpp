#pragma once

#include <openssl/types.h>
#include <string>
namespace magpie {

class SSLConfig {
private:
    std::string keyFile;
    std::string certFile;

    void createSslContext();
public:

    SSL_CTX* sslCtx = nullptr;

    SSLConfig(
        const std::string& keyFile, 
        const std::string& certFile
    );
    SSLConfig(SSLConfig&) = delete;
    SSLConfig(SSLConfig&& other):
        keyFile(std::move(other.keyFile)), 
        certFile(std::move(other.certFile)),
        sslCtx(std::move(other.sslCtx)) {
        other.sslCtx = nullptr;
    }
    ~SSLConfig();

    /**
     * Generates SSL certificates, and then initialises an SSLConfig with that generated certificate. 
     *
     * \raisewarning    This should never be used outside debug mode. This is exclusively meant to make local
     *                  development easier without needing to spin up an entire script to generate a dummy SSL cert to
     *                  make HTTP/2 happy. This function is therefore explicitly designed to limit customisability,
     *                  because its use outside development is actively discouraged.
     */
    static SSLConfig fromGeneratedCertificate();
};

}
