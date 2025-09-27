#include "client.h"
#include <string>
#include <stdexcept>
#include <unistd.h>     // write, read
#include <sys/socket.h> // send, recv
#include <cstring>
#include <sys/un.h>
#include <spdlog/spdlog.h>
#include "rpc/delimiter_codec.h"
RpcClient::RpcClient(const std::string& host, int port)
    : host(host), port(port),pool(1){
    // create socket used to connect to server
    //AF_UNIX:UNIX domain socket
    //SOCK_STREAM:stream socket(TCP)
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        throw std::runtime_error("RpcClient: failed to create socket");
    }

    // fill in client address structure
    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::string sock_path = "/tmp/mr-rpc.sock";
    std::strncpy(addr.sun_path, sock_path.c_str(), sizeof(addr.sun_path) - 1);
    //ensure null-termination
    addr.sun_path[sizeof(addr.sun_path)-1] = '\0';
    //do not unlink the socket file here
    //because the server may not be running yet

    // 连接可能在 server 尚未 bind 时失败，做重试更稳健（test 脚本里经常发生）
    int attempts = 0;
    const int max_retries = 5;
    const int retry_interval_ms = 100; // 每次重试间隔 100ms
    while (true) {
        // 使用 sizeof(addr) 通常没问题；也可以用更精确的长度：
        // socklen_t len = offsetof(struct sockaddr_un, sun_path) + strlen(addr.sun_path) + 1;
        spdlog::info("RpcClient connecting to {}:{}", host, port);
        if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            spdlog::info("RpcClient connected to {}:{} at attempt {}", host, port, attempts + 1);
            break;
        } // success
        int e = errno;
        if (++attempts > max_retries) {
            spdlog::error("RpcClient failed to connect to {}:{} after {} attempts: {}", host, port, attempts, strerror(e));
            close(sock_fd);
            throw std::runtime_error(std::string("connect() failed: ") + strerror(e));
        }
        // 常见错误：ENOENT (socket 文件不存在)、ECONNREFUSED 等 -> 等待并重试
        std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms));
    }
}
RpcClient::~RpcClient(){
    if(sock_fd >= 0)
        close(sock_fd);
}
std::string RpcClient::call(
    const std::string& method, 
    const std::string& payload){
    rpc::DelimiterCodec codec;

    // 1. constuct request payload
    const std::string request_payload = method + "\n" + payload;

    // 2. encode to framed message
    std::string framed = codec.encodeRequest(request_payload);
    spdlog::info("RpcClient sending request: method='{}', payload='{}'", method, payload);
    //3. send request
    //send parameters:connecting fd,buff,buff size,flag
    ssize_t n = send(sock_fd, framed.c_str(), framed.size(), 0);
    if (n < 0) {
        throw std::runtime_error("RpcClient::call send() failed");
    }

    //wait for response
    std::string buffer;
    char tmp[4096];
    while (true) {
        //block until some data is received
        ssize_t r = recv(sock_fd, tmp, sizeof(tmp), 0);
        if (r < 0) {
            throw std::runtime_error("RpcClient::call recv() failed");
        } else if (r == 0) {
            // server closed connection
            break;
        }
        buffer.append(tmp, r);

        // try decode
        auto resp = codec.tryDecodeResponse(buffer);
        if (resp) {
            return *resp; // return response payload
        }
    }

    return "ERROR: no response";
}