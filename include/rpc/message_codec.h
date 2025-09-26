#pragma once
#include <string>
#include <optional> // for std::optional,support from c++ 

namespace rpc {

    // a rpcrequest consists of a method name and a payload
    struct RpcRequest {
        std::string method;
        std::string payload;
    };
    struct RpcResponse{
        //only payload for now
        std::string payload;
    };
    

    // IMessageCodec: responsible for mutual conversion between RPC requests/responses and byte streams
    // tryDecodeXXX accepts a growing buffer (std::string&):
    // - if buffer contains a complete message, returns the parsed result and removes consumed bytes from buffer
    // - if not,leave the buffer untouched and return std::nullopt
    class IMessageCodec {
        public:
            virtual ~IMessageCodec() = default;

            //encode a rpcrequest structor that contains method name and payload to a string(framing)
            virtual std::string encodeRequest(const RpcRequest& req) = 0;

            //try to decode a request from buffer front
            //if successful, return RpcRequest and remove consumed data from buffer
            //if not, leave buffer untouched and return std::nullopt
            virtual std::optional<RpcRequest> tryDecodeRequest(std::string& buffer) = 0;

            //encode a rpcresponse structor or a string to response string(framing and delimeter included)
            virtual std::string encodeResponse(const std::string& payload) = 0;
            virtual std::string encodeResponse(const RpcResponse& payload) = 0;

            //try to decode a response from buffer front
            // if successful, return response payload and remove consumed data from buffer
            // if not, leave buffer untouched and return std::nullopt
            virtual std::optional<std::string> tryDecodeResponse(std::string& buffer) = 0;
    };

} // namespace rpc



/*
why base class is called IMessageCodec?
I:interface,naming convention inherited from COM(Compoent Object Model)
meaning the class is a abstract interface class,i.e. pure virtual functions only
*/