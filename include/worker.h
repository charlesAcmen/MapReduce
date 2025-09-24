#pragma once
#include "mapreduce.h"
#include "coordinator.h"

class Worker {
    public:
        Worker(Coordinator &coord, MapFunc mapf, ReduceFunc reducef);
        void run();

    private:
        Coordinator &coord;
        MapFunc mapf;
        ReduceFunc reducef;

        void doMap(const Task &task);
        void doReduce(const Task &task);
};
