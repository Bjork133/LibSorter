#include "core/storage/json_storage.hpp"
#include "common/utf8_util.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <stdexcept>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <mutex> // ВАЖНО: добавлено для std::unique_lock
#include <nlohmann/json.hpp>

using libsorter::common::utf8_tolower;

namespace libsorter::core {

std::string JsonFileStorage::generate_uuid() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 15);
    std::stringstream ss;
    for (int i = 0; i < 8; ++i)
        ss << "0123456789abcdef"[dist(rng)];
    return ss.str();
}

JsonFileStorage::JsonFileStorage(fs::path data_dir)
    : m_data_dir(std::move(data_dir)) {
    fs::create_directories(m_data_dir / "books");
    fs::create_directories(m_data_dir / "users");
    load_all_books();
    load_all_users();
}

void JsonFileStorage::load_all_books() {
    std::unique_lock lock(m_mutex);
    m_books.clear();
    auto books_dir = m_data_dir / "books";
    if (!fs::exists(books_dir)) return;
    
    for (auto& entry : fs::directory_iterator(books_dir)) {
        if (entry.path().extension() == ".json") {
            std::ifstream ifs(entry.path());
            if (ifs) {
                try {
                    nlohmann::json j;
                    ifs >> j;
                    Book b;
                    from_json(j, b);
                    m_books[b.id] = b;
                } catch (...) {
                    // Ignore corrupted files for now
                }
            }
        }
    }
}

void JsonFileStorage::load_all_users() {
    std::unique_lock lock(m_mutex);
    m_users.clear();
    auto users_dir = m_data_dir / "users";
    if (!fs::exists(users_dir)) return;

    for (auto& entry : fs::directory_iterator(users_dir)) {
        if (entry.path().extension() == ".json") {
            std::ifstream ifs(entry.path());
            if (ifs) {
                try {
                    nlohmann::json j;
                    ifs >> j;
                    User u;
                    from_json(j, u);
                    m_users[u.id] = u;
                } catch (...) {}
            }
        }
    }
}

void JsonFileStorage::write_book_atomic(const Book& book) const {
    auto path = book_path(book.id);
    auto tmp_path = path;
    tmp_path += ".tmp";
    
    std::ofstream ofs(tmp_path);
    if (!ofs) throw std::runtime_error("Failed to open temp file for writing");
    
    nlohmann::json j;
    to_json(j, book);
    ofs << j.dump(4);
    ofs.close();
    
    std::rename(tmp_path.c_str(), path.c_str());
}

void JsonFileStorage::write_user_atomic(const User& user) const {
    auto path = user_path(user.id);
    auto tmp_path = path;
    tmp_path += ".tmp";
    
    std::ofstream ofs(tmp_path);
    if (!ofs) throw std::runtime_error("Failed to open temp file for writing");
    
    nlohmann::json j;
    to_json(j, user);
    ofs << j.dump(4);
    ofs.close();
    
    std::rename(tmp_path.c_str(), path.c_str());
}

void JsonFileStorage::delete_book_file(const std::string& id) const {
    fs::remove(book_path(id));
}

void JsonFileStorage::delete_user_file(const std::string& id) const {
    fs::remove(user_path(id));
}

fs::path JsonFileStorage::book_path(const std::string& id) const {
    return m_data_dir / "books" / (id + ".json");
}

fs::path JsonFileStorage::user_path(const std::string& id) const {
    return m_data_dir / "users" / (id + ".json");
}

Book JsonFileStorage::add(Book book) {
    book.id = generate_uuid();
    book.created_at = book.updated_at = std::chrono::system_clock::now();
    {
        std::unique_lock lock(m_mutex);
        m_books[book.id] = book;
    }
    write_book_atomic(book);
    return book;
}

std::optional<Book> JsonFileStorage::get(const std::string& id) const {
    std::shared_lock lock(m_mutex);
    auto it = m_books.find(id);
    if (it == m_books.end()) return std::nullopt;
    return it->second;
}

bool JsonFileStorage::update(const Book& book) {
    {
        std::unique_lock lock(m_mutex);
        auto it = m_books.find(book.id);
        if (it == m_books.end()) return false;
        it->second = book;
        it->second.updated_at = std::chrono::system_clock::now();
    }
    write_book_atomic(book);
    return true;
}

bool JsonFileStorage::remove(const std::string& id) {
    {
        std::unique_lock lock(m_mutex);
        if (m_books.erase(id) == 0) return false;
    }
    delete_book_file(id);
    return true;
}

std::vector<Book> JsonFileStorage::list(const Filter& filter) const {
    std::shared_lock lock(m_mutex);
    std::vector<Book> result;
    for (const auto& [_, book] : m_books) {
        if (filter.matches(book)) {
            result.push_back(book);
        }
    }
    return result;
}

size_t JsonFileStorage::count() const {
    std::shared_lock lock(m_mutex);
    return m_books.size();
}

void JsonFileStorage::reload() {
    load_all_books();
    load_all_users();
}

void JsonFileStorage::flush() {
    // No-op for file-based storage, writes are immediate
}

User JsonFileStorage::add_user(User user) {
    user.id = generate_uuid();
    user.created_at = user.updated_at = std::chrono::system_clock::now();
    {
        std::unique_lock lock(m_mutex);
        m_users[user.id] = user;
    }
    write_user_atomic(user);
    return user;
}

std::optional<User> JsonFileStorage::get_user(const std::string& id) const {
    std::shared_lock lock(m_mutex);
    auto it = m_users.find(id);
    if (it == m_users.end()) return std::nullopt;
    return it->second;
}

std::optional<User> JsonFileStorage::get_user_by_username(const std::string& username) const {
    std::shared_lock lock(m_mutex);
    for (const auto& [_, user] : m_users) {
        if (user.username == username) return user;
    }
    return std::nullopt;
}

bool JsonFileStorage::update_user(const User& user) {
    {
        std::unique_lock lock(m_mutex);
        auto it = m_users.find(user.id);
        if (it == m_users.end()) return false;
        it->second = user;
        it->second.updated_at = std::chrono::system_clock::now();
    }
    write_user_atomic(user);
    return true;
}

bool JsonFileStorage::remove_user(const std::string& id) {
    {
        std::unique_lock lock(m_mutex);
        if (m_users.erase(id) == 0) return false;
    }
    delete_user_file(id);
    return true;
}

std::vector<User> JsonFileStorage::list_users() const {
    std::shared_lock lock(m_mutex);
    std::vector<User> result;
    for (const auto& [_, user] : m_users) {
        result.push_back(user);
    }
    return result;
}

bool Filter::matches(const Book& book) const {
    if (status && book.status != *status) return false;
    
    if (genre) {
        if (!book.genre || *book.genre != *genre) return false;
    }
    
    if (author) {
        bool found = false;
        std::string lower_author = utf8_tolower(*author);
        for (const auto& a : book.authors) {
            if (utf8_tolower(a).find(lower_author) != std::string::npos) {
                found = true;
                break;
            }
        }
        if (!found) return false;
    }
    
    if (query) {
        std::string lower_query = utf8_tolower(*query);
        bool found = utf8_tolower(book.title).find(lower_query) != std::string::npos;
        if (!found) {
            for (const auto& a : book.authors) {
                if (utf8_tolower(a).find(lower_query) != std::string::npos) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) return false;
    }
    
    return true;
}

} // namespace libsorter::core
