#include "server/handlers/auth_handler.hpp"
#include "common/crypto.hpp"

namespace libsorter::server {
using namespace libsorter::common;

AuthHandler::AuthHandler(AuthMiddleware& auth, const std::string& library_code, libsorter::core::JsonFileStorage& storage)
    : m_auth(auth), m_library_code(library_code), m_storage(storage) {}

WsMessage AuthHandler::handle_connect(const WsMessage& req) {
    std::string username = req.payload.value("username", "");
    std::string password = req.payload.value("password", "");
    std::string code = req.payload.value("code", "");

    if (username.empty() || password.empty() || code.empty())
        return make_error_response(req.id, ErrorCode::BadRequest, "Missing username, password or code");

    if (code != m_library_code)
        return make_error_response(req.id, ErrorCode::Forbidden, "Invalid library code");

    // Ищем пользователя
    auto user_opt = m_storage.get_user_by_username(username);
    
    if (!user_opt.has_value()) {
        // Авто-регистрация нового пользователя (Reader)
        libsorter::core::User new_user;
        new_user.username = username;
        new_user.salt = generate_salt();
        new_user.password_hash = hash_password(password, new_user.salt);
        new_user.role = libsorter::core::Role::Reader;
        
        m_storage.add_user(new_user);
        
        std::string token = m_auth.create_session(username);
        return make_response(req.id, {{"token", token}, {"role", "reader"}});
    }

    // Проверка пароля существующего пользователя
    auto& user = *user_opt;
    if (!verify_password(password, user.salt, user.password_hash))
        return make_error_response(req.id, ErrorCode::Unauthorized, "Invalid password");

    std::string token = m_auth.create_session(username);
    return make_response(req.id, {{"token", token}, {"role", role_to_string(user.role)}});
}

} // namespace libsorter::server
