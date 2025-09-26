#pragma once
#include <string>
#include "ThreadPool.h"

/*
RPC message format is defined as:
[methodName]\n[payload]\nEND\n
*/


struct RpcRequest {
    //method name in string:key in handlers_(unordered_map)
    std::string method;
    std::string payload;
};


// 客户端 RPC
class RpcClient {
public:
    //hose:server ip address
    //port:server port
    RpcClient(const std::string& host, int port);
    //call rpc by method name and pass payload
    std::string call(const std::string& method, const std::string& payload);
private:
    std::string host;
    int port;
    ThreadPool pool;
};