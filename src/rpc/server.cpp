#include "server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>//sockaddr_un
#include <unistd.h>//close()
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <spdlog/spdlog.h>
#include <thread>
#include "rpc/delimiter_codec.h"
RpcServer::RpcServer(int port) : port(port){}
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
    std::strncpy(addr.sun_path, sock_path.c_str(), sizeof(addr.sun_path) - 1);
    //ensure null-termination
    addr.sun_path[sizeof(addr.sun_path)-1] = '\0';
    //cpy will copy until '\0',but sun_path is not guaranteed to be null-terminated
    // strcpy(addr.sun_path, sock_path.c_str());
    //delete existing socket file,or Address already in use error
    ::unlink(sock_path.c_str()); 

    //bind() will create the socket file at sock_path automatically
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        spdlog::error("Failed to bind socket");
        close(server_fd);
        throw std::runtime_error("Failed to bind socket");
    }
    //listen for incoming connections
    if (listen(server_fd, SOMAXCONN) < 0) {
        spdlog::error("Failed to listen on socket");
        close(server_fd);
        throw std::runtime_error("Failed to listen on socket");
    }

    spdlog::info("RpcServer listening on {}", sock_path);

    //in loop to accept and handle connections
    while (true) {
        //block until a new connection is accepted
        int client_fd = accept(server_fd, nullptr, nullptr);
        if(client_fd < 0) {
            spdlog::error("Failed to accept connection");
            //continue to stay alive
            continue;
        }
        spdlog::info("Accepted new connection: fd={}", client_fd);
        //do not enqueue here,because rpc client has long connection with rpc server
        // pool.enqueue([this, client_fd]() {
        //use detached thread instead
        std::thread([this, client_fd]() {
            rpc::DelimiterCodec codec;
            char buf[4096];
            std::string data;
            //ssize_t: signed size type
            //recv parameters: socket fd, buffer, buffer size, flags
            //flags:0 means no special options
            //:: avoids potential c library function name conflicts with member functions
            while(true){
                ssize_t n = ::recv(client_fd, buf, sizeof(buf), 0);
                //rpc is once time request-response, so only recv once
                if (n == 0) {
                    // client closed normally
                    spdlog::info("Client closed connection");
                    close(client_fd);
                    return;
                } else if (n < 0) {
                    spdlog::error("recv failed: {}", strerror(errno));
                    close(client_fd);
                    return;
                }
                data.append(buf, n);
                //now data should conform to the rpc message format defined in task.h
                //[methodName]\n[payload]\nEND\n
                //there could be multiple rpc messages in data
                auto req_opt = codec.tryDecodeRequest(data);
                //req_opt is std::optional<RpcRequest>
                while (req_opt) {
                    const auto& [method, payload] = *req_opt;

                    // === call handler ===
                    std::string reply_payload;
                    spdlog::info("Received RPC request: method='{}', payload='{}'", method, payload);
                    if (handlers.count(method)) {
                        reply_payload = handlers[method](payload);
                    } else {
                        reply_payload = "ERROR: unknown method";
                    }

                    // === encode response ===
                    std::string framed = codec.encodeResponse(reply_payload);
                    // spdlog::info("Sending RPC response: {}'", framed);
                    ::send(client_fd, framed.data(), framed.size(), 0);

                    // update opt
                    req_opt = codec.tryDecodeRequest(data);
                }
                //do not close until all messages are processed
            }
            close(client_fd);
            spdlog::info("Closed connection: fd={}", client_fd);
        // });
        }).detach();
    }
}