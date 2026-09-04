#pragma once
#include <string>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>

namespace libsorter::common {

inline std::string sha256_hex(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, input.data(), input.size());
    SHA256_Final(hash, &ctx);
    
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setfill('0') << std::setw(2) << (int)hash[i];
    return ss.str();
}

inline std::string generate_salt() {
    unsigned char buf[16];
    RAND_bytes(buf, sizeof(buf));
    std::stringstream ss;
    for (int i = 0; i < 16; ++i)
        ss << std::hex << std::setfill('0') << std::setw(2) << (int)buf[i];
    return ss.str();
}

inline std::string hash_password(const std::string& password, const std::string& salt) {
    return sha256_hex(password + salt);
}

inline bool verify_password(const std::string& password, const std::string& salt, const std::string& hash) {
    return hash_password(password, salt) == hash;
}

} // namespace libsorter::common
