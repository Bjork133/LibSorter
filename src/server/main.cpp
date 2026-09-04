#include "server/ws_server.hpp"
#include "core/storage/json_storage.hpp"
#include "common/crypto.hpp"
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>

// Функция для скрытого ввода пароля в консоли
std::string get_password() {
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    std::string password;
    std::getline(std::cin, password);
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    std::cout << "\n";
    return password;
}

int main(int argc, char* argv[]) {
    std::string data_dir = "./data";
    uint16_t port = 9443;

    if (argc > 1) data_dir = argv[1];
    if (argc > 2) port = static_cast<uint16_t>(std::stoi(argv[2]));

    // Проверяем, есть ли уже пользователи (первый запуск?)
    libsorter::core::JsonFileStorage storage(data_dir);
    if (storage.list_users().empty()) {
        std::cout << "[LibSorter] First run detected. No users found.\n";
        std::cout << "Please create the Host account.\n";
        
        std::string username;
        std::cout << "Enter Host username: ";
        std::getline(std::cin, username);
        
        std::cout << "Enter Host password: ";
        std::string password = get_password();
        
        std::cout << "Confirm password: ";
        std::string confirm = get_password();
        
        if (password != confirm) {
            std::cerr << "Passwords do not match. Exiting.\n";
            return 1;
        }
        
        if (username.empty() || password.empty()) {
            std::cerr << "Username and password cannot be empty. Exiting.\n";
            return 1;
        }

        // Создаем хоста
        libsorter::core::User host;
        host.username = username;
        host.salt = libsorter::common::generate_salt();
        host.password_hash = libsorter::common::hash_password(password, host.salt);
        host.role = libsorter::core::Role::Admin;
        host.can_view_logs = true;
        host.can_view_stats = true;
        
        storage.add_user(host);
        std::cout << "[LibSorter] Host account '" << username << "' created successfully.\n";
    }

    try {
        libsorter::server::WsServer server(data_dir, port);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
