#pragma once
#include "server/auth_middleware.hpp"
#include "server/handlers/book_handler.hpp"
#include "server/handlers/auth_handler.hpp"
#include "core/storage/json_storage.hpp"
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <string>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace libsorter::server {

class WsSession : public std::enable_shared_from_this<WsSession> {
public:
    WsSession(tcp::socket socket, BookHandler& bh, AuthHandler& ah);
    void run();

private:
    void on_accept(beast::error_code ec);
    void on_read(beast::error_code ec, size_t bytes_transferred);
    void handle_message(std::string_view msg);

    websocket::stream<tcp::socket> m_ws;
    beast::flat_buffer m_buffer;
    BookHandler& m_book_handler;
    AuthHandler& m_auth_handler;
};

class WsServer {
public:
    WsServer(const std::string& data_dir, uint16_t port);
    void run();

private:
    void do_accept();
    static std::string load_or_create_library_code(const std::filesystem::path& data_dir);

    std::unique_ptr<libsorter::core::JsonFileStorage> m_storage;
    AuthMiddleware m_auth;
    std::string m_library_code;
    std::unique_ptr<BookHandler> m_book_handler;
    std::unique_ptr<AuthHandler> m_auth_handler;
    
    net::io_context m_ioc;
    tcp::acceptor m_acceptor;
    uint16_t m_port;
};

} // namespace libsorter::server
