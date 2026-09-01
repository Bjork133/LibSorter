#pragma once
#include "core/models/book.hpp"
#include "core/models/user.hpp"
#include <filesystem>
#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

namespace libsorter::core {

struct Filter {
    std::optional<std::string> query;
    std::optional<std::string> author;
    std::optional<std::string> genre;
    std::optional<BookStatus> status;
    bool matches(const Book& book) const;
};

class JsonFileStorage {
public:
    explicit JsonFileStorage(fs::path data_dir);
    
    Book add(Book book);
    std::optional<Book> get(const std::string& id) const;
    bool update(const Book& book);
    bool remove(const std::string& id);
    std::vector<Book> list(const Filter& filter = {}) const;
    size_t count() const;
    void reload();
    void flush();

    User add_user(User user);
    std::optional<User> get_user(const std::string& id) const;
    std::optional<User> get_user_by_username(const std::string& username) const;
    bool update_user(const User& user);
    bool remove_user(const std::string& id);
    std::vector<User> list_users() const;

private:
    fs::path m_data_dir;
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, Book> m_books;
    std::unordered_map<std::string, User> m_users;

    void load_all_books();
    void load_all_users();
    void write_book_atomic(const Book& book) const;
    void write_user_atomic(const User& user) const;
    void delete_book_file(const std::string& id) const;
    void delete_user_file(const std::string& id) const;
    fs::path book_path(const std::string& id) const;
    fs::path user_path(const std::string& id) const;
    static std::string generate_uuid();
};

} // namespace libsorter::core
