#include "SSLConfig.hpp"
#include <cstdio>
#include <filesystem>
#include <openssl/asn1.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <stdexcept>

namespace magpie {

SSLConfig::SSLConfig(const std::string& keyFile, const std::string& certFile) 
    : keyFile(keyFile), certFile(certFile) {
}

SSLConfig SSLConfig::fromGeneratedCertificate() {
    static const std::filesystem::path sslRoot = "./test-certificates/";
    static const std::filesystem::path 
        key = sslRoot / "key.pem",
        cert = sslRoot / "cert.pem";

    std::filesystem::create_directories(sslRoot);

    if (!std::filesystem::exists(key) && !std::filesystem::exists(cert)) {
        EVP_PKEY* privateKey = EVP_PKEY_new();
        auto* pkeyCtx = EVP_PKEY_CTX_new_id(
            EVP_PKEY_ED25519,
            nullptr
        );

        if (!pkeyCtx) {
            // TODO: Real error handling (copy from the other function)
            // ... or do I just not care?
            throw std::runtime_error("Looks like something went badly");
        }

        if (EVP_PKEY_keygen_init(pkeyCtx) <= 0) {
            // TODO: Real error handling (copy from the other function)
            throw std::runtime_error("Looks like something went badly");
        }

        if (EVP_PKEY_keygen(pkeyCtx, &privateKey) <= 0) {
            // TODO: Real error handling (copy from the other function)
            throw std::runtime_error("Looks like something went badly");
        }

        auto* x509 = X509_new();

        ASN1_INTEGER_set(X509_get_serialNumber(x509), 69);

        X509_gmtime_adj(X509_get_notBefore(x509), 0);
        X509_gmtime_adj(X509_get_notAfter(x509), 60 * 60 * 24 * 365 * 10); // 10 years from now

        X509_set_pubkey(x509, privateKey);
        auto name = X509_get_subject_name(x509);
        X509_NAME_add_entry_by_txt(
            name,
            "C",
            MBSTRING_ASC,
            (unsigned char*) "NO", -1, -1, 0
        );
        X509_NAME_add_entry_by_txt(
            name,
            "O",
            MBSTRING_ASC,
            (unsigned char*) "codeberg:LunarWatcher/magpie", -1, -1, 0
        );
        // TODO: would be nice if this also worked with IP out of the box. 
        // https://stackoverflow.com/a/41366949 suggests there's a nice way to do it, but it's CLI-based rather than
        // C-based and 5 minutes failed to translate it
        X509_NAME_add_entry_by_txt(
            name,
            "CN",
            MBSTRING_ASC,
            (unsigned char*) "localhost", -1, -1, 0
        );

        X509_set_issuer_name(
            x509, name
        );

        if (!X509_sign(
            x509,
            privateKey,
            nullptr
        )) {
            
            throw std::runtime_error(std::string {
                ERR_error_string(
                    ERR_get_error(), nullptr
                )
            });
        }

        // Pretty sure this is standard C: https://en.cppreference.com/w/c/io/fopen
        FILE* keyFilePtr = fopen(key.c_str(), "wb");
        FILE* certFilePtr = fopen(cert.c_str(), "wb");

        PEM_write_PrivateKey(
            keyFilePtr,
            privateKey,
            nullptr, // encryption method (disabled)
            nullptr, // password (disabled, for obvious reasons)
            0, // password length
            nullptr, nullptr // callback shit?
        );

        PEM_write_X509(
            certFilePtr, x509
        );

        fclose(keyFilePtr);
        fclose(certFilePtr);

        EVP_PKEY_free(privateKey);
        EVP_PKEY_CTX_free(pkeyCtx);

        X509_free(x509);
    }

    // Converting straight to a c_str because std::filesystem::path is weird some times
    return SSLConfig(key.c_str(), cert.c_str());
}

}
