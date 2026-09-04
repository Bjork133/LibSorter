#include "server/ws_server.hpp"
#include "common/protocol.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <fstream>      // <--- ДЛЯ РАБОТЫ С ФАЙЛАМИ
#include <filesystem>   // <--- ДЛЯ ПУТЕЙ
#include <nlohmann/json.hpp>

namespace libsorter::server {
using namespace libsorter::common;
using json = nlohmann::json;
namespace fs = std::filesystem;

static std::string generate_library_code() {
    static const char charset[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, sizeof(charset) - 2);
    std::string code;
    for (int i = 0; i < 8; ++i)
        code += charset[dist(rng)];
    return code;
}

// <--- НОВАЯ ФУНКЦИЯ: ЗАГРУЗКА ИЛИ СОЗДАНИЕ КОДА
static std::string load_or_create_library_code(const fs::path& data_dir) {
    // Убедимся, что директория существует
    fs::create_directories(data_dir);
    
    auto code_file = data_dir / "library_code.txt";
    
    // 1. Пробуем прочитать существующий код
    if (fs::exists(code_file)) {
        std::ifstream ifs(code_file);
        if (ifs.is_open()) {
            std::string code;
            if (std::getline(ifs, code) && code.size() == 8) {
                std::cout << "[LibSorter] Loaded existing library code from " << code_file << "\n";
                return code;
            }
        }
    }
    
    // 2. Если файла нет или он битый — генерируем новый
    std::cout << "[LibSorter] Generating new library code...\n";
    std::string code = generate_library_code();
    
    // 3. Сохраняем в файл
    std::ofstream ofs(code_file);
    if (ofs.is_open()) {
        ofs << code;
        ofs.close();
        std::cout << "[LibSorter] Saved new library code to " << code_file << "\n";
    } else {
        std::cerr << "[LibSorter] ERROR: Could not save library code to " << code_file << "\n";
    }
    
    return code;
}

WsSession::WsSession(tcp::socket socket,
                     BookHandler& bh, AuthHandler& ah)
    : m_ws(std::move(socket))
    , m_book_handler(bh)
    , m_auth_handler(ah) {}

void WsSession::run() {
    auto self = shared_from_this();
    m_ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
    m_ws.async_accept([self](beast::error_code ec) { self->on_accept(ec); });
}

void WsSession::on_accept(beast::error_code ec) {
    if (ec) { std::cerr << "[WS] Accept error: " << ec.message() << "\n"; return; }
    std::cout << "[WS] Client connected\n";
    auto self = shared_from_this();
    m_ws.async_read(m_buffer, [self](beast::error_code ec, size_t bytes) { self->on_read(ec, bytes); });
}

void WsSession::on_read(beast::error_code ec, size_t) {
    if (ec == websocket::error::closed) {
        std::cout << "[WS] Client disconnected\n";
        return;
    }
    if (ec) { std::cerr << "[WS] Read error: " << ec.message() << "\n"; return; }
    std::string msg = beast::buffers_to_string(m_buffer.data());
    m_buffer.consume(m_buffer.size());
    handle_message(msg);
    auto self = shared_from_this();
    m_ws.async_read(m_buffer, [self](beast::error_code ec, size_t bytes) { self->on_read(ec, bytes); });
}

void WsSession::handle_message(std::string_view msg) {
    try {
        json incoming = json::parse(msg);
        WsMessage req = WsMessage::from_json(incoming);
        WsMessage resp;
        if (req.action == "auth.connect") {
            resp = m_auth_handler.handle_connect(req);
        } else if (req.action.starts_with("book.")) {
            if (!req.token)
                resp = make_error_response(req.id, ErrorCode::Unauthorized, "Token required");
            else
                resp = m_book_handler.handle(req);
        } else {
            resp = make_error_response(req.id, ErrorCode::BadRequest, "Unknown action");
        }
        std::string out = resp.to_json().dump();
        m_ws.text(true);
        m_ws.write(net::buffer(out));
    } catch (const std::exception& e) {
        std::string err = make_error_response("", ErrorCode::InternalError, e.what()).to_json().dump();
        m_ws.text(true);
        m_ws.write(net::buffer(err));
    }
}

WsServer::WsServer(const std::string& data_dir, uint16_t port)
    : m_storage(std::make_unique<libsorter::core::JsonFileStorage>(data_dir))
    , m_auth()
    , m_library_code(load_or_create_library_code(data_dir)) // <--- ИСПОЛЬЗУЕМ НОВУЮ ФУНКЦИЮ
    , m_book_handler(std::make_unique<BookHandler>(*m_storage, m_auth))
    , m_auth_handler(std::make_unique<AuthHandler>(m_auth, m_library_code, *m_storage))
    , m_ioc{std::max(1u, std::thread::hardware_concurrency())}
    , m_acceptor(m_ioc)
    , m_port(port) {}

void WsServer::run() {
    tcp::endpoint endpoint(tcp::v4(), m_port);
    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(net::socket_base::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen(net::socket_base::max_listen_connections);

    std::cout << "[LibSorter] Starting on port " << m_port << "\n"
              << "[LibSorter] Library code: " << m_library_code << "\n"
              << "[LibSorter] Listening...\n";

    do_accept();

    std::vector<std::thread> threads;
    unsigned int n = std::max(1u, std::thread::hardware_concurrency()) - 1;
    for (unsigned int i = 0; i < n; ++i)
        threads.emplace_back([this] { m_ioc.run(); });
    m_ioc.run();
    for (auto& t : threads) t.join();
}

void WsServer::do_accept() {
    m_acceptor.async_accept(
        [this](beast::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::make_shared<WsSession>(
                    std::move(socket), *m_book_handler, *m_auth_handler
                )->run();
            } else {
                std::cerr << "[WS] Accept failed: " << ec.message() << "\n";
            }
            do_accept();
        });
}

} // namespace libsorter::server
