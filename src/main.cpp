#include "coordinator.h"
#include "worker.h"
#include <thread>
#include <spdlog/spdlog.h>
#include <sstream>
// #include <string>
#include <fstream> 
#include <iostream>

std::vector<std::string> splitWords(const std::string& text) {
    std::vector<std::string> words;
    std::string current;
    for (char c : text) {
        // Consider only alphabetic characters as part of words
        if (std::isalpha(static_cast<unsigned char>(c))) {
            current.push_back(c);
        } else {
            if (!current.empty()) {
                // If we hit a non-alphabetic character and have a current word, save it
                words.push_back(current);
                current.clear();
            }
        }
    }
    if (!current.empty()) words.push_back(current);
    return words;
}



// wordcount map/reduce
std::vector<KeyValue> wcMap(const std::string &filename, const std::string &content) {
    std::vector<KeyValue> res;
    auto words = splitWords(content);
    for (auto& w : words) {
        res.push_back({w, "1"});
    }
    return res;
}

std::string wcReduce(const std::string &key, const std::vector<std::string> &values) {
    return std::to_string(values.size());
}
//global nReduce,which serves as the number of reduce tasks
//the higher the nReduce is, the more intermediate files will be generated,and the faster the reduce phase will be
const int nReduce = 3;

int main() {
    std::vector<std::string> files = 
    {
        "pg-being_ernest.txt", 
        "pg-dorian_gray.txt",
        "pg-frankenstein.txt",
        "pg-grimm.txt",
        "pg-huckleberry_finn.txt",
        "pg-metamorphosis.txt",
        "pg-sherlock_holmes.txt",
        "pg-tom_sawyer.txt"
    };
    Coordinator coord(files, nReduce);

    std::vector<std::thread> workers;
    for (int i = 0; i < nReduce+1; i++) {
        workers.emplace_back([&]() {
            Worker w(coord, wcMap, wcReduce);
            w.run();
        });
    }
    for (auto &t : workers) t.join();
    spdlog::info("All tasks done!");

    std::stringstream finalBuffer;
    for (int i = 0; i < nReduce+1; i++) {
        std::ifstream in("./mr-out/mr-out-" +std::to_string(i));
        // Read entire file content
        finalBuffer << in.rdbuf();
    }
    //sort the final output by alphabetical order,no need to utilize map
    



    std::ofstream out("./mr-out/mr-final.txt", std::ios::out | std::ios::trunc);
    out << finalBuffer.str();
    spdlog::info("Final output written to ./mr-out/mr-final.txt");
    return 0;
}
