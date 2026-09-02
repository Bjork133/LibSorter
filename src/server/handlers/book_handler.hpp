#pragma once
#include "core/storage/json_storage.hpp"
#include "server/auth_middleware.hpp"
#include "common/protocol.hpp"

namespace libsorter::server {

class BookHandler {
public:
    BookHandler(libsorter::core::JsonFileStorage& storage, AuthMiddleware& auth);
    common::WsMessage handle(const common::WsMessage& req);

private:
    libsorter::core::JsonFileStorage& m_storage;
    AuthMiddleware& m_auth;

    common::WsMessage handle_list(const common::WsMessage& req);
    common::WsMessage handle_get(const common::WsMessage& req);
    common::WsMessage handle_add(const common::WsMessage& req);
    common::WsMessage handle_update(const common::WsMessage& req);
    common::WsMessage handle_remove(const common::WsMessage& req);
    common::WsMessage handle_borrow(const common::WsMessage& req);
    common::WsMessage handle_return(const common::WsMessage& req);
};

} // namespace libsorter::server
