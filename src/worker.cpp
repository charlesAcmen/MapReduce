#include "worker.h"
#include <fstream>
#include <sstream>
#include <map>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

Worker::Worker(Coordinator &coord, MapFunc mapf, ReduceFunc reducef)
    : coord(coord), mapf(mapf), reducef(reducef) {}

void Worker::run() {
    //while true?thread pool?
    while (true) {
        Task task{TaskType::None, -1, "", TaskState::Idle};
        if (!coord.getTask(task)) {
            // No task available
            if (coord.done()) 
                // All tasks are done
                break;
            // Wait and retry
            continue;
        }
        if (task.type == TaskType::Map) {
            doMap(task);
            // Report map task completion
            coord.reportDone(task.id, TaskType::Map);
        } else if (task.type == TaskType::Reduce) {
            doReduce(task);
            // Report reduce task completion
            coord.reportDone(task.id, TaskType::Reduce);
        }
    }
}

void Worker::doMap(const Task &task) {
    std::ifstream in("./data/" + task.filename);
    std::stringstream buffer;
    // Read entire file content
    buffer << in.rdbuf();
    // Apply map function
    //kvs: vector of <key, value> pairs
    auto kvs = mapf(task.filename, buffer.str());

    int nReduce = coord.getNReduce();
    // Open intermediate files for each reduce task
    std::vector<std::ofstream> outFiles(nReduce);
    for (int i = 0; i < nReduce; i++) {
        //mr-X-Y where X is the map task id and Y is the reduce task id
        outFiles[i].open(
            "./mr-intermediate/mr-" + std::to_string(task.id) + "-" + std::to_string(i),
            //output & truncate if exists
            std::ios::out | std::ios::trunc);
    }
    for (auto &kv : kvs) {
        // Hash key to determine the reduce task index
        int idx = ihash(kv.first, nReduce);
        //seprate key and value with a space, each key-value pair in a new line
        outFiles[idx] << kv.first << " " << kv.second << "\n";
    }
}

void Worker::doReduce(const Task &task) {
    // Aggregate intermediate key-value pairs
    std::map<std::string, std::vector<std::string>> kvMap;
    //directory_iterator iterates over files in the current directory
    //"./data/" represents the text directory
    for (auto &p : fs::directory_iterator("./mr-intermediate/")) {
        // Get filename
        std::string name = p.path().filename().string();
        //rfind("mr-", 0) checks if the string starts with "mr-"
        //back() gets the last character of the string
        //''0' + task.id'' converts task.id to its corresponding character
        if (name.rfind("mr-", 0) == 0 && name.back() == '0' + task.id) {
            std::ifstream in(
                "./mr-intermediate/" + name);
            std::string key, value;
            //each kv pair is in a new line, key and value are separated by a space
            while (in >> key >> value) {
                kvMap[key].push_back(value);
            }
        }
    }
    // Apply reduce function and write to output file
    //mr-out-X where X is the reduce task id
    std::ofstream out(
        "./mr-out/mr-out-" + std::to_string(task.id),
        std::ios::out | std::ios::trunc);
    for (auto &[key, values] : kvMap) {
        // Apply reduce function
        std::string res = reducef(key, values);
        out << key << " " << res << "\n";
    }
}
