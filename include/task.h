#pragma once
#include <string>
enum class TaskState { Idle, InProgress, Completed };
enum class TaskType { Map, Reduce, None };

inline std::string to_string(TaskType type) {
    switch (type) {
        case TaskType::Map: return "Map";
        case TaskType::Reduce: return "Reduce";
        case TaskType::None: return "None";
    }
    return "UNKNOWN";
}

inline std::string to_string(TaskState state) {
    switch (state) {
        case TaskState::Idle: return "Idle";
        case TaskState::InProgress: return "InProgress";
        case TaskState::Completed: return "Completed";
    }
    return "UNKNOWN";
}



struct Task {
    TaskType type;
    //in range of 0--nReduce-1--len(files)-1
    int id;
    //each input file corresponds to a map task
    std::string filename;
    TaskState state;
};