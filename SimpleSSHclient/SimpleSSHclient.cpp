#include <libssh2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

LIBSSH2_SESSION* ConnectSsh(const std::string& hostname, int port, const std::string& username, const std::string& password) {
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 0), &wsadata) != 0) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return nullptr;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return nullptr;
    }

    sockaddr_in sin{};
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    inet_pton(AF_INET, hostname.c_str(), &(sin.sin_addr));

    if (connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to the server" << std::endl;
        closesocket(sock);
        WSACleanup();
        return nullptr;
    }

    libssh2_init(0);

    LIBSSH2_SESSION* session = libssh2_session_init();
    if (!session) {
        std::cerr << "Failed to initialize libssh2 session" << std::endl;
        closesocket(sock);
        WSACleanup();
        return nullptr;
    }

    if (libssh2_session_handshake(session, sock)) {
        std::cerr << "Failed to establish SSH session" << std::endl;
        libssh2_session_free(session);
        closesocket(sock);
        WSACleanup();
        return nullptr;
    }

    if (libssh2_userauth_password(session, username.c_str(), password.c_str()) != 0) {
        std::cerr << "Authentication failed" << std::endl;
        libssh2_session_disconnect(session, "Bye bye, cruel world");
        libssh2_session_free(session);
        closesocket(sock);
        WSACleanup();
        return nullptr;
    }

    std::cout << "Authentication successful" << std::endl;

    return session;
}

void DisconnectSsh(LIBSSH2_SESSION* session) {
    libssh2_session_disconnect(session, "Bye bye, cruel world");
    libssh2_session_free(session);
    libssh2_exit();
}

int main() {
    std::string hostname = "server";
    std::string username = "user";
    std::string password = "password";
    int port = 22;

    LIBSSH2_SESSION* session = ConnectSsh(hostname, port, username, password);
    if (!session) {
        return -1;
    }

    LIBSSH2_CHANNEL* channel = libssh2_channel_open_session(session);
    if (channel) {
        std::cout << "Executing command on remote server" << std::endl;
        libssh2_channel_exec(channel, "touch first_step.txt");

        libssh2_channel_send_eof(channel);
        libssh2_channel_wait_eof(channel);
        libssh2_channel_wait_closed(channel);
        libssh2_channel_free(channel);
    }
    else {
        std::cerr << "Failed to open channel" << std::endl;
    }

    DisconnectSsh(session);

    return 0;
}
