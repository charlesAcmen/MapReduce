#pragma once
#include <string>
#include <functional>
#include <unordered_map>
// #include "ThreadPool.h"

/*
RPC response format is defined as:
[response payload]\nEND\n
*/

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
    //return type : std::string, param type: const std::string& as payload
    std::unordered_map<std::string, std::function<std::string(const std::string&)>> handlers;

    //do not use thread pool here, because each rpc client has long connection with rpc server
    //use detached thread instead
    // ThreadPool pool;
};
