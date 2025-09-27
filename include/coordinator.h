#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "rpc/server.h"
#include "mapreduce.h"
#include "task.h"

class Coordinator {
    public:
        //lists of input files, number of reduce tasks
        Coordinator(const std::vector<std::string> &files, int nReduce=3,int nWorkers=3);
        // get an idle task, return false if no task available
        bool getTask(Task &task);
        // mark task as done
        void reportDone(int taskId, TaskType type);
        // return true if all reduce tasks are completed
        bool done();

        int getNReduce() const { return nReduce; }

        void run();
    private:
        std::vector<Task> mapTasks;
        std::vector<Task> reduceTasks;
        int nReduce;
        std::mutex mtx;
        RpcServer rpcServer;

        std::atomic<int> activeWorkers; // number of active workers
};
