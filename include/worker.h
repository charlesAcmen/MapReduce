#pragma once
#include "mapreduce.h"
#include "coordinator.h"
#include "rpc/client.h"
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
