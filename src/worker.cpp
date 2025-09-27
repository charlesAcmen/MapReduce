#include "worker.h"
#include <fstream>  // for std::ifstream, std::ofstream
#include <sstream>  // for std::stringstream
#include <map>  // for std::map kvmap
#include <filesystem>   // for std::filesystem
#include <spdlog/spdlog.h>
#include "rpc/delimiter_codec.h"  // for rpc::DelimiterCodec


namespace fs = std::filesystem;

Worker::Worker( MapFunc mapf, ReduceFunc reducef)
    :  mapf(mapf), reducef(reducef),rpcClient(){}

void Worker::run() {
    while (true) {
        Task task{TaskType::None, -1, "", TaskState::Idle};
        std::string reply = rpcClient.call("RequestTask", "");
        if (reply == "NoTask") {
            spdlog::info("No more tasks available, worker {} exiting...", getpid());
            rpcClient.call("WorkerExit", "");
            break;
        }



        std::istringstream iss(reply);
        std::string typeStr, stateStr;
        if (!(iss >> typeStr)) {
            spdlog::error("Failed to parse task type: {}", reply);
            continue;
        }

        if (typeStr == "NoTask") {
            spdlog::info("No more tasks available, worker exiting...");
            break;
        } 

        task.type = taskTypeFromString(typeStr);

        if (task.type == TaskType::Map) {
            if (!(iss >> task.id >> task.filename >> stateStr)) {
                spdlog::error("Failed to parse Map task: {}", reply);
                continue;
            }
        } else if (task.type == TaskType::Reduce) {
            if (!(iss >> task.id >> stateStr)) {
                spdlog::error("Failed to parse Reduce task: {}", reply);
                continue;
            }
            task.filename = ""; // Reduce tasks do not have filenames
        }

        task.state = taskStateFromString(stateStr);
        // spdlog::info("Received task: type={}, id={}, filename={}, state={}",to_string(task.type), task.id, task.filename, to_string(task.state));

        if (task.type == TaskType::Map) {
            doMap(task);
            // Report map task completion
            rpcClient.call("ReportDone",std::to_string(task.id) + "\n" + "Map");
            // coord.reportDone(task.id, TaskType::Map);
        } else if (task.type == TaskType::Reduce) {
            doReduce(task);
            // Report reduce task completion
            rpcClient.call("ReportDone", std::to_string(task.id) + "\n" + "Reduce");
            // coord.reportDone(task.id, TaskType::Reduce);
        }else {
            // None or Idle：sleep to avoid busy loop
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void Worker::doMap(const Task &task) {
    spdlog::info("Worker starting map task id={}, filename={}", task.id, task.filename);
    std::ifstream in("./data/" + task.filename);
    std::stringstream buffer;
    // Read entire file content
    buffer << in.rdbuf();
    // Apply map function
    //kvs: vector of <key, value> pairs
    auto kvs = mapf(task.filename, buffer.str());

    // Get number of reduce tasks from coordinator
    int nReduce = std::stoi(rpcClient.call("GetNReduce", ""));
    // int nReduce = coord.getNReduce();
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
    spdlog::info("Worker starting reduce task id={}", task.id);
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
