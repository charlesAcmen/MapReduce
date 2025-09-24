#include "coordinator.h"
#include "worker.h"
#include <thread>
#include <spdlog/spdlog.h>
#include <sstream>
// wordcount map/reduce
std::vector<KeyValue> wcMap(const std::string &filename, const std::string &content) {
    std::vector<KeyValue> res;
    std::istringstream iss(content);
    std::string word;
    while (iss >> word) {
        res.push_back({word, "1"});
    }
    return res;
}

std::string wcReduce(const std::string &key, const std::vector<std::string> &values) {
    return std::to_string(values.size());
}

int main() {
    std::vector<std::string> files = 
    {
        "pg-being_ernest.txt", 
        "pg-dorian_gray.txt",
    };
    Coordinator coord(files, 3);

    std::vector<std::thread> workers;
    for (int i = 0; i < 4; i++) {
        workers.emplace_back([&]() {
            Worker w(coord, wcMap, wcReduce);
            w.run();
        });
    }
    for (auto &t : workers) t.join();
    spdlog::info("All tasks done!");
}
