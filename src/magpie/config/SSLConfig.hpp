#pragma once

#include <openssl/types.h>
#include <string>

namespace magpie {

class SSLConfig {
public:
    std::string keyFile;
    std::string certFile;
    SSLConfig(
        const std::string& keyFile, 
        const std::string& certFile
    );
    SSLConfig(SSLConfig&) = delete;
    SSLConfig(SSLConfig&& other):
        keyFile(std::move(other.keyFile)), 
        certFile(std::move(other.certFile)) {
    }

    SSLConfig& operator=(SSLConfig&& other) {
        this->keyFile = std::move(other.keyFile);
        this->certFile = std::move(other.certFile);
        return *this;
    }

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
