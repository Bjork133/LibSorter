#include "server/tls_util.hpp"
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace libsorter::server {
namespace fs = std::filesystem;

static std::string compute_fingerprint(X509* cert) {
    unsigned char der_buf[8192];
    unsigned char* p = der_buf;
    int der_len = i2d_X509(cert, &p);
    if (der_len <= 0) return "unknown";
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, der_buf, der_len);
    EVP_DigestFinal_ex(ctx, md, &len);
    EVP_MD_CTX_free(ctx);
    std::ostringstream oss;
    for (unsigned int i = 0; i < len; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)md[i] << ":";
    std::string fp = oss.str();
    if (!fp.empty()) fp.pop_back();
    return fp;
}

TlsConfig ensure_tls_cert(const fs::path& certs_dir) {
    TlsConfig config;
    config.cert_path = (certs_dir / "server.crt").string();
    config.key_path  = (certs_dir / "server.key").string();

    if (fs::exists(config.cert_path) && fs::exists(config.key_path)) {
        FILE* f = fopen(config.cert_path.c_str(), "r");
        if (f) {
            X509* cert = PEM_read_X509(f, nullptr, nullptr, nullptr);
            fclose(f);
            if (cert) { config.fingerprint_sha256 = compute_fingerprint(cert); X509_free(cert); }
        }
        return config;
    }

    fs::create_directories(certs_dir);
    EVP_PKEY* pkey = EVP_PKEY_new();
    RSA* rsa = RSA_new();
    BIGNUM* e = BN_new();
    BN_set_word(e, RSA_F4);
    RSA_generate_key_ex(rsa, 2048, e, nullptr);
    EVP_PKEY_assign_RSA(pkey, rsa);
    BN_free(e);

    X509* x509 = X509_new();
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    X509_gmtime_adj(X509_getm_notBefore(x509), 0);
    X509_gmtime_adj(X509_getm_notAfter(x509), 365L * 24 * 3600 * 10);
    X509_set_pubkey(x509, pkey);
    X509_NAME* name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"LibSorter Server", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_sha256());

    FILE* cf = fopen(config.cert_path.c_str(), "w");
    PEM_write_X509(cf, x509); fclose(cf);
    FILE* kf = fopen(config.key_path.c_str(), "w");
    PEM_write_PrivateKey(kf, pkey, nullptr, nullptr, 0, nullptr, nullptr); fclose(kf);

    config.fingerprint_sha256 = compute_fingerprint(x509);
    std::cout << "[TLS] Generated self-signed certificate\n"
              << "[TLS] SHA256 Fingerprint: " << config.fingerprint_sha256 << "\n";

    X509_free(x509);
    EVP_PKEY_free(pkey);
    return config;
}

} // namespace libsorter::server
