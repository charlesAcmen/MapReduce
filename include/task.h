#pragma once
#include <string>
#include <stdexcept>
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

inline TaskType taskTypeFromString(const std::string& s) {
    if (s == "Map") return TaskType::Map;
    if (s == "Reduce") return TaskType::Reduce;
    if (s == "None") return TaskType::None;
    throw std::invalid_argument("Unknown TaskType string: " + s);
}

inline TaskState taskStateFromString(const std::string& s) {
    if (s == "Idle") return TaskState::Idle;
    if (s == "InProgress") return TaskState::InProgress;
    if (s == "Completed") return TaskState::Completed;
    throw std::invalid_argument("Unknown TaskState string: " + s);
}


struct Task {
    TaskType type;
    //in range of 0--nReduce-1--len(files)-1
    int id;
    //each input file corresponds to a map task
    std::string filename;
    TaskState state;
};