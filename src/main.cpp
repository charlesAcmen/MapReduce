#include "coordinator.h"
#include "worker.h"
#include <thread>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
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

bool areFilesIdentical(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1, 
        //binary: prevent newline translation on Windows
        //ate: open and seek to the end of the file,usage: to get file size
        std::ios::binary|std::ios::ate);
    std::ifstream f2(file2, 
        std::ios::binary|std::ios::ate);
    
    if (!f1.is_open() || !f2.is_open()) {
        spdlog::error("Failed to open one of the files: {} or {}", file1, file2);
        return false;
    }
    // auto size1 = f1.tellg();
    // auto size2 = f2.tellg();
    // if (size1 != size2) {
    //     spdlog::warn("File sizes differ: {} ({} bytes) vs {} ({} bytes)", 
    //                  file1, size1, file2, size2);
    //     return false;
    // }
    
    // 重置到文件开头
    f1.seekg(0, std::ios::beg);
    f2.seekg(0, std::ios::beg);
    
    //buffer comparison: memory efficient for large files
    constexpr std::size_t bufferSize = 4096;
    std::vector<char> buf1(bufferSize), buf2(bufferSize);

    while (f1 && f2) {
        f1.read(buf1.data(), bufferSize);
        f2.read(buf2.data(), bufferSize);

        // Number of bytes read
        auto bytesRead1 = f1.gcount();
        auto bytesRead2 = f2.gcount();

        if (bytesRead1 != bytesRead2 || 
            //parameter: first1, last1, first2
            !std::equal(buf1.begin(), buf1.begin() + bytesRead1, buf2.begin())) {
            spdlog::warn("Files differ at offset {}", f1.tellg());
            return false;
        }
    }
    
    return true;
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
    for (int i = 0; i < nReduce; i++) {
        workers.emplace_back([&]() {
            Worker w(coord, wcMap, wcReduce);
            w.run();
        });
    }
    for (auto &t : workers) t.join();
    spdlog::info("All tasks done!");

   std::vector<std::string> lines;
    for (int i = 0; i < nReduce; i++) {
        std::ifstream in("./mr-out/mr-out-" +std::to_string(i));
        // Read entire file content
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }
        in.close();
    }

    std::sort(lines.begin(), lines.end());
    std::ofstream out("./mr-out/mr-final.txt",

        std::ios::out | std::ios::trunc | std::ios::binary);
    for (const auto& sortedLine : lines) {
        out << sortedLine << "\n";
    }

    
    spdlog::info("Final output written to ./mr-out/mr-final.txt");

    if (areFilesIdentical("data/answers/mr-correct-wc.txt", "./mr-out/mr-final.txt")) {
        spdlog::info("Output is correct and matches mr-wc.txt");
    } else {
        spdlog::error("Output does not match mr-wc.txt");
    }

    return 0;
}
