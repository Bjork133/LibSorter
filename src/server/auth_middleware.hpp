#pragma once
#include "core/models/user.hpp"
#include <string>
#include <unordered_map>
#include <optional>
#include <mutex>

namespace libsorter::server {

using libsorter::core::Role;

class AuthMiddleware {
public:
    std::string create_session(const std::string& username);
    void register_user(const std::string& username, Role role, const std::string& library_code);
    bool is_valid(const std::string& token) const;
    std::optional<Role> get_role(const std::string& token) const;
    bool has_permission(const std::string& token, Role required) const;
    void invalidate(const std::string& token);

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, Role> m_sessions;
    
    // Вспомогательная функция для генерации токена
    static std::string generate_token();
};

} // namespace libsorter::server
