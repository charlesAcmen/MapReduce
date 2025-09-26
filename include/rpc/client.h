#pragma once
#include <string>
#include "ThreadPool.h"

/*
RPC message format is defined as:
[methodName]\n[payload]\nEND\n
*/


// struct RpcRequest {
//     //method name in string type:key in handlers(unordered_map)
//     std::string method;
//     std::string payload;
// };


// 客户端 RPC
class RpcClient {
public:
    //hose:server ip address
    //port:server port
    RpcClient(const std::string& host = "127.0.0.1", int port = 12345);
    ~RpcClient();
    //call rpc by method name and pass payload
    std::string call(const std::string& method, const std::string& payload);
private:
    std::string host;
    int port;
    //accept() will return a new socket fd called client_fd for communication
    //it is the socket that serves to send and receive data
    //sock_fd is the socket fd used to connect to the server
    int sock_fd{-1};
    ThreadPool pool;
};