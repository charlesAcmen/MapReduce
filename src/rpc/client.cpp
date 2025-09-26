#include "client.h"
RpcClient::RpcClient(const std::string& host, int port)
    : host(host), port(port),pool(1){}
std::string RpcClient::call(
    const std::string& method, 
    const std::string&){
    return "";
}