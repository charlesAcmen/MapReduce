#include "coordinator.h"

Coordinator::Coordinator(const std::vector<std::string> &files, int nReduce) 
    : nReduce(nReduce),rpcServer(12345){
    for (int i = 0; i < (int)files.size(); i++) {
        mapTasks.push_back({TaskType::Map, i, files[i], TaskState::Idle});
    }
    for (int i = 0; i < nReduce; i++) {
        reduceTasks.push_back({TaskType::Reduce, i, "", TaskState::Idle});
    }
}
/*
scheule map tasks first, then reduce tasks
just for simplicity
*/
bool Coordinator::getTask(Task &task) {
    std::lock_guard<std::mutex> lk(mtx);
    //first assign map tasks if possible
    for (auto &t : mapTasks) {
        if (t.state == TaskState::Idle) {
            t.state = TaskState::InProgress;
            task = t;
            return true;
        }
    }
    bool allMapDone = true;
    for (auto &t : mapTasks) if (t.state != TaskState::Completed) allMapDone = false;
    if (allMapDone) {
        //then assign reduce tasks
        for (auto &t : reduceTasks) {
            if (t.state == TaskState::Idle) {
                t.state = TaskState::InProgress;
                task = t;
                return true;
            }
        }
    }
    return false;
}

void Coordinator::reportDone(int taskId, TaskType type) {
    std::lock_guard<std::mutex> lk(mtx);
    if (type == TaskType::Map) mapTasks[taskId].state = TaskState::Completed;
    else if (type == TaskType::Reduce) reduceTasks[taskId].state = TaskState::Completed;
}

bool Coordinator::done() {
    std::lock_guard<std::mutex> lk(mtx);
    for (auto &t : reduceTasks) {
        if (t.state != TaskState::Completed) return false;
    }
    return true;
}
