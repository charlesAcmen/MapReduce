#pragma once
#include "mapreduce.h"  // for MapFunc, ReduceFunc
#include "coordinator.h" // for Task
#include "rpc/client.h" // for RpcClient
class Worker {
    public:
        Worker(MapFunc mapf, ReduceFunc reducef);
        void run();

    private:
        MapFunc mapf;
        ReduceFunc reducef;
        RpcClient rpcClient;

        void doMap(const Task &task);
        void doReduce(const Task &task);
};
