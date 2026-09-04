#pragma once
#include <string>
#include <filesystem>

namespace libsorter::server {

struct TlsConfig {
    std::string cert_path;
    std::string key_path;
    std::string fingerprint_sha256;
};

TlsConfig ensure_tls_cert(const std::filesystem::path& certs_dir);

} // namespace libsorter::server
