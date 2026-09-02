#pragma once
#include "server/auth_middleware.hpp"
#include "core/storage/json_storage.hpp"
#include "common/protocol.hpp"

namespace libsorter::server {

class AuthHandler {
public:
    AuthHandler(AuthMiddleware& auth, const std::string& library_code, libsorter::core::JsonFileStorage& storage);
    common::WsMessage handle_connect(const common::WsMessage& req);

private:
    AuthMiddleware& m_auth;
    std::string m_library_code;
    libsorter::core::JsonFileStorage& m_storage;
};

} // namespace libsorter::server
