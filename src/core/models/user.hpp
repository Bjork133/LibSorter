#pragma once
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

namespace libsorter::core {

enum class Role {
    Admin,
    Librarian,
    Reader
};

inline std::string role_to_string(Role r) {
    switch (r) {
        case Role::Admin: return "admin";
        case Role::Librarian: return "librarian";
        case Role::Reader: return "reader";
    }
    return "reader";
}

inline Role role_from_string(const std::string& s) {
    if (s == "admin") return Role::Admin;
    if (s == "librarian") return Role::Librarian;
    return Role::Reader;
}

struct User {
    std::string id;
    std::string username;
    std::string password_hash;
    std::string salt;
    Role role = Role::Reader;
    bool can_view_logs = false;
    bool can_view_stats = false;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};

inline void to_json(nlohmann::json& j, const User& u) {
    j = nlohmann::json::object();
    j["id"] = u.id;
    j["username"] = u.username;
    j["password_hash"] = u.password_hash;
    j["salt"] = u.salt;
    j["role"] = role_to_string(u.role);
    j["can_view_logs"] = u.can_view_logs;
    j["can_view_stats"] = u.can_view_stats;
    j["created_at"] = std::chrono::system_clock::to_time_t(u.created_at);
    j["updated_at"] = std::chrono::system_clock::to_time_t(u.updated_at);
}

inline void from_json(const nlohmann::json& j, User& u) {
    j.at("id").get_to(u.id);
    j.at("username").get_to(u.username);
    j.at("password_hash").get_to(u.password_hash);
    j.at("salt").get_to(u.salt);
    if (j.contains("role")) 
        u.role = role_from_string(j.at("role").get<std::string>());
    if (j.contains("can_view_logs")) 
        j.at("can_view_logs").get_to(u.can_view_logs);
    if (j.contains("can_view_stats")) 
        j.at("can_view_stats").get_to(u.can_view_stats);
    if (j.contains("created_at"))
        u.created_at = std::chrono::system_clock::from_time_t(j.at("created_at").get<time_t>());
    if (j.contains("updated_at"))
        u.updated_at = std::chrono::system_clock::from_time_t(j.at("updated_at").get<time_t>());
}

} // namespace libsorter::core
