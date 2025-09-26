#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include "ThreadPool.h"

/*
RPC response format is defined as:
[response payload]\nEND\n
*/

// struct RpcResponse {
//     std::string payload;
// };

class RpcServer {
public:
    RpcServer(int port=12345);
    //register rpc handler by method name
    void register_handler(const std::string& method,
                          std::function<std::string(const std::string&)> handler);
    void start();
private:
    //server port
    int port;
    //return type : std::string, param type: const std::string&
    std::unordered_map<std::string, std::function<std::string(const std::string&)>> handlers;

    ThreadPool pool;
};
