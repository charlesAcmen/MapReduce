#include "server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>//sockaddr_un
#include <unistd.h>//close()
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <spdlog/spdlog.h>
RpcServer::RpcServer(int port) : port(port),pool(1){
}
void RpcServer::register_handler(
    const std::string& method,
    std::function<std::string(const std::string&)> handler) {
    handlers[method] = handler;
}
/*
UNIX Domain Socket(AF_UNIX):bind() requires a file path, not an IP address and port.
1. Create a socket with socket(AF_UNIX, SOCK_STREAM, 0).
2. Bind the socket to a socket file path using bind().
3. Listen for incoming connections with listen().
4. Accept connections with accept().
5. Communicate using send() and recv().
6. Close the socket with close().
*/
void RpcServer::start() {
    //AF_UNIX:UNIX domain socket
    //SOCK_STREAM:stream socket
    //0:default protocol for AF_UNIX and SOCK_STREAM is 0
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(server_fd < 0) {
        spdlog::error("Failed to create socket");
        return;
    }

    //sockaddr_un: UNIX domain socket address structure
    sockaddr_un addr{};
    //sun_family:address family(AF_UNIX)
    //un stands for: UNIX domain
    addr.sun_family = AF_UNIX;
    std::string sock_path = "/tmp/mr-rpc.sock";
    //strcpy is unsafe, use strncpy instead
    //sun_path:socket file path
    //sock_path.c_str():convert std::string to const char*
    strncpy(addr.sun_path, sock_path.c_str(), sizeof(addr.sun_path) - 1);
    // strcpy(addr.sun_path, sock_path.c_str());
    //delete existing socket file,or Address already in use error
    unlink(sock_path.c_str()); 

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        spdlog::error("Failed to bind socket");
        close(server_fd);
        return;
    }
    if (listen(server_fd, SOMAXCONN) < 0) {
        spdlog::error("Failed to listen on socket");
        close(server_fd);
        return;
    }

    spdlog::info("RpcServer listening on {}", sock_path);

    //in loop to accept and handle connections
    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if(client_fd < 0) {
            spdlog::error("Failed to accept connection");
            continue;
        }
        pool.enqueue([this, client_fd]() {
            char buf[4096];
            std::string data;
            //ssize_t: signed size type
            ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
            if (n <= 0) return;
            data.append(buf, n);

            // 按 END\n 分割
            size_t pos;
            while ((pos = data.find("\nEND\n")) != std::string::npos) {
                std::string msg = data.substr(0, pos);
                data.erase(0, pos + 5);

                // 解析 method + payload
                std::istringstream iss(msg);
                std::string method;
                std::getline(iss, method);
                std::string payload((std::istreambuf_iterator<char>(iss)),
                                    std::istreambuf_iterator<char>());

                // 调用 handler
                std::string reply;
                if (handlers.count(method)) {
                    reply = handlers[method](payload);
                } else {
                    reply = "ERROR: unknown method";
                }

                reply += "\nEND\n";
                send(client_fd, reply.c_str(), reply.size(), 0);
            }
            close(client_fd);
        });
    }
}