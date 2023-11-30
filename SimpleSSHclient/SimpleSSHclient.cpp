#include <libssh2.h>
#include <ws2tcpip.h>
#include <iostream>

// Подключение библиотеки ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

int main() {
    std::string hostname = "server";
    std::string username = "user";
    std::string password = "password";
    int port = 22;
    SOCKET sock = INVALID_SOCKET;
    WSADATA wsadata;
    struct sockaddr_in sin;
    LIBSSH2_SESSION* session;

    // Инициализация Winsock
    WSAStartup(MAKEWORD(2, 0), &wsadata);

    // Создание сокета
    sock = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    inet_pton(AF_INET, hostname.c_str(), &(sin.sin_addr));

    connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in));

    // Инициализация библиотеки libssh2
    libssh2_init(0);

    // Создание новой сессии
    session = libssh2_session_init();

    // Подключение к серверу SSH
    libssh2_session_handshake(session, sock);

    // Аутентификация
    if (libssh2_userauth_password(session, username.c_str(), password.c_str()) == 0) {
        std::cout << "Authentication successful" << std::endl;

        // Выполнение команды SSH
        LIBSSH2_CHANNEL* channel = libssh2_channel_open_session(session);
        if (channel) {
            std::cout << "Executing command on remote server" << std::endl;

            libssh2_channel_exec(channel, "touch first_step.txt");

            // Закрытие канала
            libssh2_channel_send_eof(channel);
            libssh2_channel_wait_eof(channel);
            libssh2_channel_wait_closed(channel);
            libssh2_channel_free(channel);
        }
        else {
            std::cerr << "Failed to open channel" << std::endl;
        }
    }
    else {
        std::cerr << "Authentication failed" << std::endl;
    }

    // Закрытие сессии
    libssh2_session_disconnect(session, "Bye bye, cruel world");
    libssh2_session_free(session);

    // Закрытие сокета
    closesocket(sock);

    // Очистка библиотеки libssh2
    libssh2_exit();

    // Завершение Winsock
    WSACleanup();

    return 0;
}