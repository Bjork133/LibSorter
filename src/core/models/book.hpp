#pragma once
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <nlohmann/json.hpp>

namespace libsorter::core {

enum class BookStatus {
    Available,
    Borrowed,
    Lost
};

inline std::string status_to_string(BookStatus s) {
    switch (s) {
        case BookStatus::Available: return "available";
        case BookStatus::Borrowed:  return "borrowed";
        case BookStatus::Lost:      return "lost";
    }
    return "available";
}

inline BookStatus status_from_string(const std::string& s) {
    if (s == "borrowed") return BookStatus::Borrowed;
    if (s == "lost") return BookStatus::Lost;
    return BookStatus::Available;
}

struct Book {
    std::string id;
    std::string title;
    std::vector<std::string> authors;
    int year = 0;
    std::optional<std::string> genre;
    int pages = 0;
    int weight = 0; // Вес в граммах
    std::string location_id;
    BookStatus status = BookStatus::Available;
    std::string borrowed_by;
    std::optional<std::chrono::system_clock::time_point> borrowed_at;
    std::vector<std::string> tags;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};

inline void to_json(nlohmann::json& j, const Book& b) {
    j = nlohmann::json::object(); // Явное создание объекта
    j["id"] = b.id;
    j["title"] = b.title;
    j["authors"] = b.authors;
    j["year"] = b.year;
    j["pages"] = b.pages;
    j["weight"] = b.weight;
    j["location_id"] = b.location_id;
    j["status"] = status_to_string(b.status);
    j["borrowed_by"] = b.borrowed_by;
    j["tags"] = b.tags;
    j["created_at"] = std::chrono::system_clock::to_time_t(b.created_at);
    j["updated_at"] = std::chrono::system_clock::to_time_t(b.updated_at);

    if (b.genre.has_value())
        j["genre"] = *b.genre;
    else
        j["genre"] = nullptr;

    if (b.borrowed_at.has_value())
        j["borrowed_at"] = std::chrono::system_clock::to_time_t(*b.borrowed_at);
    else
        j["borrowed_at"] = nullptr;
}

inline void from_json(const nlohmann::json& j, Book& b) {
    j.at("id").get_to(b.id);
    j.at("title").get_to(b.title);
    j.at("authors").get_to(b.authors);
    j.at("year").get_to(b.year);
    j.at("pages").get_to(b.pages);
    
    if (j.contains("weight")) 
        j.at("weight").get_to(b.weight);
    
    if (j.contains("location_id")) 
        j.at("location_id").get_to(b.location_id);
        
    if (j.contains("status")) 
        b.status = status_from_string(j.at("status").get<std::string>());
        
    if (j.contains("borrowed_by")) 
        j.at("borrowed_by").get_to(b.borrowed_by);
        
    if (j.contains("tags")) 
        j.at("tags").get_to(b.tags);

    if (j.contains("genre") && !j.at("genre").is_null())
        b.genre = j.at("genre").get<std::string>();
    else
        b.genre = std::nullopt;

    if (j.contains("borrowed_at") && !j.at("borrowed_at").is_null())
        b.borrowed_at = std::chrono::system_clock::from_time_t(j.at("borrowed_at").get<time_t>());
    else
        b.borrowed_at = std::nullopt;

    if (j.contains("created_at"))
        b.created_at = std::chrono::system_clock::from_time_t(j.at("created_at").get<time_t>());
    if (j.contains("updated_at"))
        b.updated_at = std::chrono::system_clock::from_time_t(j.at("updated_at").get<time_t>());
}

} // namespace libsorter::core
