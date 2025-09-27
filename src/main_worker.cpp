#include <vector>
#include <string>
#include <cctype>// for std::isalpha
#include "mapreduce.h"
#include "worker.h"
#include <spdlog/spdlog.h>

#include <fstream>        // std::ifstream, std::ofstream  // 用于文件读写
#include <algorithm>      // std::sort  // 用于按字母排序
#include <filesystem>     // std::filesystem::directory_iterator  // 遍历目录

namespace fs = std::filesystem;



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

int main() {
    Worker worker(wcMap, wcReduce);
    worker.run();


    spdlog::info("All tasks done!");

    std::vector<std::string> lines;

    // iterate through  ./mr-out/ 
    for (const auto& entry : fs::directory_iterator("./mr-out/")) {
        if (!entry.is_regular_file()) continue;
        std::string filename = entry.path().filename().string();

        // all mr-out-* files
        if (filename.find("mr-out-") != 0) continue;

        std::ifstream in(entry.path(), std::ios::binary);
        if (!in) {
            spdlog::error("Failed to open file: {}", entry.path().string());
            continue;
        }

        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        in.close();
    }

    //sort by alphabetical order
    std::sort(lines.begin(), lines.end());

    //write to mr-wc-all.txt
    std::ofstream out("./mr-out/mr-wc-all.txt",
                      std::ios::out | std::ios::trunc | std::ios::binary);
    for (const auto& sortedLine : lines) {
        out << sortedLine << "\n";
    }
    out.close();

    spdlog::info("Final output written to ./mr-out/mr-wc-all.txt");
    return 0;
}
