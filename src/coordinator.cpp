#include "coordinator.h"
#include "delimiter_codec.h"
#include <spdlog/spdlog.h>
#include <sstream>
Coordinator::Coordinator(const std::vector<std::string> &files, int nReduce) 
    : nReduce(nReduce),rpcServer(){
    for (int i = 0; i < (int)files.size(); i++) {
        mapTasks.push_back({TaskType::Map, i, files[i], TaskState::Idle});
    }
    for (int i = 0; i < nReduce; i++) {
        reduceTasks.push_back({TaskType::Reduce, i, "", TaskState::Idle});
    }
    //register rpc handlers
    //1. RequestTask
    rpcServer.register_handler("RequestTask", 
        [this](const std::string &payload) -> std::string{
            Task task{TaskType::None, -1, "", TaskState::Idle};
            if (this->getTask(task)) {
                //serialize task to string
                // format: type\nid\nfilename\nstate\n
                std::string s = to_string(task.type) + "\n" +
                                std::to_string(task.id) + "\n" +
                                task.filename + "\n" +
                                to_string(task.state);
                
                return s;
            } else {
                return std::string("NoTask");
            }
        });
    //2. ReportDone
    rpcServer.register_handler("ReportDone",
        [this](const std::string &payload) -> std::string{
            //parse payload: taskId(int)\ntaskType(string)
            std::istringstream iss(payload);
            int taskId;
            std::string typeStr;

            if (!(iss >> taskId >> typeStr)) {
                spdlog::error("ReportDone: failed to parse payload: {}", payload);
                return "error";
            }

            
            TaskType type = taskTypeFromString(typeStr); 
            this->reportDone(taskId, type);
            return "ok";
        });
    //3. GetNReduce
    rpcServer.register_handler("GetNReduce", 
        [this](const std::string &payload) -> std::string {
        return std::to_string(this->getNReduce());
    });
}
/*
schedule map tasks first, then reduce tasks
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
        spdlog::info("All map tasks completed, now assigning reduce tasks");
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

void Coordinator::run() {
    rpcServer.start();
}
