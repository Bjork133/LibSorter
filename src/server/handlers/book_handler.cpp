#include "server/handlers/book_handler.hpp"
#include "common/protocol.hpp"

namespace libsorter::server {
using namespace libsorter::common;
using libsorter::core::Book;
using libsorter::core::Filter;
using libsorter::core::Role;
using libsorter::core::BookStatus;

// Локальная helper-функция, так как она inline в book.hpp, но иногда линковщик капризничает
static BookStatus status_from_string(const std::string& s) {
    if (s == "available") return BookStatus::Available;
    if (s == "borrowed") return BookStatus::Borrowed;
    if (s == "lost") return BookStatus::Lost;
    return BookStatus::Available;
}

BookHandler::BookHandler(libsorter::core::JsonFileStorage& storage, AuthMiddleware& auth)
    : m_storage(storage), m_auth(auth) {}

WsMessage BookHandler::handle(const WsMessage& req) {
    if (req.action == "book.list")   return handle_list(req);
    if (req.action == "book.get")    return handle_get(req);
    if (req.action == "book.add")    return handle_add(req);
    if (req.action == "book.update") return handle_update(req);
    if (req.action == "book.remove") return handle_remove(req);
    if (req.action == "book.borrow") return handle_borrow(req);
    if (req.action == "book.return") return handle_return(req);
    return make_error_response(req.id, ErrorCode::BadRequest, "Unknown book action");
}

WsMessage BookHandler::handle_list(const WsMessage& req) {
    Filter filter;
    if (req.payload.contains("query"))
        filter.query = req.payload.at("query").get<std::string>();
    if (req.payload.contains("author"))
        filter.author = req.payload.at("author").get<std::string>();
    if (req.payload.contains("genre"))
        filter.genre = req.payload.at("genre").get<std::string>();
    if (req.payload.contains("status"))
        filter.status = status_from_string(req.payload.at("status").get<std::string>());

    auto books = m_storage.list(filter);
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& b : books) {
        nlohmann::json j;
        to_json(j, b);
        arr.push_back(j);
    }
    return make_response(req.id, {{"books", arr}});
}

WsMessage BookHandler::handle_get(const WsMessage& req) {
    std::string id = req.payload.value("id", "");
    if (id.empty()) return make_error_response(req.id, ErrorCode::BadRequest, "Missing id");
    auto book = m_storage.get(id);
    if (!book) return make_error_response(req.id, ErrorCode::NotFound, "Book not found");
    nlohmann::json j;
    to_json(j, *book);
    return make_response(req.id, j);
}

WsMessage BookHandler::handle_add(const WsMessage& req) {
    if (!req.token || !m_auth.has_permission(*req.token, Role::Librarian))
        return make_error_response(req.id, ErrorCode::Forbidden, "Insufficient permissions");

    Book book;
    book.title = req.payload.value("title", "");
    if (req.payload.contains("authors"))
        for (auto& a : req.payload.at("authors")) book.authors.push_back(a.get<std::string>());
    book.year = req.payload.value("year", 0);
    if (req.payload.contains("genre")) book.genre = req.payload.at("genre").get<std::string>();
    book.pages = req.payload.value("pages", 0);
    book.weight = req.payload.value("weight", 0);
    if (req.payload.contains("location")) book.location_id = req.payload.at("location").get<std::string>(); // Исправлено location -> location_id
    
    if (book.title.empty() || book.authors.empty())
        return make_error_response(req.id, ErrorCode::BadRequest, "Title and authors required");

    auto saved = m_storage.add(std::move(book));
    nlohmann::json j;
    to_json(j, saved);
    return make_response(req.id, j);
}

WsMessage BookHandler::handle_update(const WsMessage& req) {
    if (!req.token || !m_auth.has_permission(*req.token, Role::Librarian))
        return make_error_response(req.id, ErrorCode::Forbidden, "Insufficient permissions");

    std::string id = req.payload.value("id", "");
    if (id.empty()) return make_error_response(req.id, ErrorCode::BadRequest, "Missing id");
    auto existing = m_storage.get(id);
    if (!existing) return make_error_response(req.id, ErrorCode::NotFound, "Book not found");

    Book updated = *existing;
    if (req.payload.contains("title")) updated.title = req.payload.at("title").get<std::string>();
    if (req.payload.contains("authors")) {
        updated.authors.clear();
        for (auto& a : req.payload.at("authors")) updated.authors.push_back(a.get<std::string>());
    }
    if (req.payload.contains("year")) updated.year = req.payload.at("year").get<int>();
    if (req.payload.contains("genre")) updated.genre = req.payload.at("genre").get<std::string>();
    if (req.payload.contains("pages")) updated.pages = req.payload.at("pages").get<int>();
    if (req.payload.contains("weight")) updated.weight = req.payload.at("weight").get<int>();
    if (req.payload.contains("location")) updated.location_id = req.payload.at("location").get<std::string>(); // Исправлено

    if (!m_storage.update(updated))
        return make_error_response(req.id, ErrorCode::InternalError, "Update failed");

    nlohmann::json j;
    to_json(j, updated);
    return make_response(req.id, j);
}

WsMessage BookHandler::handle_remove(const WsMessage& req) {
    if (!req.token || !m_auth.has_permission(*req.token, Role::Librarian))
        return make_error_response(req.id, ErrorCode::Forbidden, "Insufficient permissions");

    std::string id = req.payload.value("id", "");
    if (id.empty()) return make_error_response(req.id, ErrorCode::BadRequest, "Missing id");
    if (!m_storage.remove(id))
        return make_error_response(req.id, ErrorCode::NotFound, "Book not found");

    return make_response(req.id, {{"deleted", true}});
}

WsMessage BookHandler::handle_borrow(const WsMessage& req) {
    if (!req.token || !m_auth.has_permission(*req.token, Role::Librarian))
        return make_error_response(req.id, ErrorCode::Forbidden, "Insufficient permissions");

    std::string book_id = req.payload.value("id", "");
    std::string user_id = req.payload.value("user_id", "");

    if (book_id.empty() || user_id.empty())
        return make_error_response(req.id, ErrorCode::BadRequest, "Missing book id or user id");

    auto book_opt = m_storage.get(book_id);
    if (!book_opt)
        return make_error_response(req.id, ErrorCode::NotFound, "Book not found");

    Book book = *book_opt;
    if (book.status != BookStatus::Available)
        return make_error_response(req.id, ErrorCode::BadRequest, "Book is not available");

    book.status = BookStatus::Borrowed;
    book.borrowed_by = user_id;
    book.borrowed_at = std::chrono::system_clock::now();

    if (!m_storage.update(book))
        return make_error_response(req.id, ErrorCode::InternalError, "Failed to update book");

    nlohmann::json j;
    to_json(j, book);
    return make_response(req.id, j);
}

WsMessage BookHandler::handle_return(const WsMessage& req) {
    if (!req.token || !m_auth.has_permission(*req.token, Role::Librarian))
        return make_error_response(req.id, ErrorCode::Forbidden, "Insufficient permissions");

    std::string book_id = req.payload.value("id", "");
    if (book_id.empty())
        return make_error_response(req.id, ErrorCode::BadRequest, "Missing book id");

    auto book_opt = m_storage.get(book_id);
    if (!book_opt)
        return make_error_response(req.id, ErrorCode::NotFound, "Book not found");

    Book book = *book_opt;
    if (book.status != BookStatus::Borrowed)
        return make_error_response(req.id, ErrorCode::BadRequest, "Book is not borrowed");

    book.status = BookStatus::Available;
    book.borrowed_by = "";
    book.borrowed_at = std::nullopt;

    if (!m_storage.update(book))
        return make_error_response(req.id, ErrorCode::InternalError, "Failed to update book");

    nlohmann::json j;
    to_json(j, book);
    return make_response(req.id, j);
}

} // namespace libsorter::server
