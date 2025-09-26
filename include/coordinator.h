#pragma once
#include "mapreduce.h"
#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "rpc/server.h"
enum class TaskState { Idle, InProgress, Completed };
enum class TaskType { Map, Reduce, None };

struct Task {
    TaskType type;
    //in range of 0--nReduce-1--len(files)-1
    int id;
    //each input file corresponds to a map task
    std::string filename;
    TaskState state;
};

class Coordinator {
    public:
        //lists of input files, number of reduce tasks
        //nReduce in the 论文 is 
        Coordinator(const std::vector<std::string> &files, int nReduce);
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
};
