#include "server/auth_middleware.hpp"
#include <random>
#include <sstream>
#include <iomanip>

namespace libsorter::server {

std::string AuthMiddleware::generate_token() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
    std::stringstream ss;
    for (int i = 0; i < 4; ++i)
        ss << std::hex << std::setfill('0') << std::setw(8) << dist(rng);
    return ss.str();
}

void AuthMiddleware::register_user(const std::string&, Role, const std::string&) {
    // MVP: регистрация пока не используется напрямую, пользователи создаются через storage
}

std::string AuthMiddleware::create_session(const std::string& username) {
    std::lock_guard lock(m_mutex);
    std::string token = generate_token();
    // В MVP всем выдаем Admin для простоты тестирования, 
    // в реальности нужно брать роль из User
    m_sessions[token] = Role::Admin; 
    return token;
}

bool AuthMiddleware::is_valid(const std::string& token) const {
    std::lock_guard lock(m_mutex);
    return m_sessions.find(token) != m_sessions.end();
}

std::optional<Role> AuthMiddleware::get_role(const std::string& token) const {
    std::lock_guard lock(m_mutex);
    auto it = m_sessions.find(token);
    if (it == m_sessions.end()) return std::nullopt;
    return it->second;
}

bool AuthMiddleware::has_permission(const std::string& token, Role required) const {
    auto role = get_role(token);
    if (!role.has_value()) return false;
    // Admin (0) <= Librarian (1) <= Reader (2)
    // Если требуемая роль "выше" (число больше) или равна текущей, то доступ есть
    // Но обычно логика обратная: Admin может всё.
    // Пусть: Admin=0, Librarian=1, Reader=2.
    // has_permission(token, Librarian) -> true если роль Admin или Librarian.
    // значит current_role <= required_role
    return static_cast<int>(*role) <= static_cast<int>(required);
}

void AuthMiddleware::invalidate(const std::string& token) {
    std::lock_guard lock(m_mutex);
    m_sessions.erase(token);
}

} // namespace libsorter::server
