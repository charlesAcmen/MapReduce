#include <vector>
#include <string>
#include "coordinator.h"
#include <spdlog/spdlog.h>
#include <fstream>        // std::ifstream, std::ofstream  // 用于文件读写
#include <algorithm>      // std::sort  // 用于按字母排序
#include <filesystem>     // std::filesystem::directory_iterator  // 遍历目录

namespace fs = std::filesystem;
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
    const int nReduce = 3;
    Coordinator coord(files, nReduce);
    coord.run();





    spdlog::info("All tasks done!");

    std::vector<std::string> lines;

    // iterate through  ./mr-out/ 
    for (const auto& entry : fs::directory_iterator("./mr-out/")) {
        if (!entry.is_regular_file()) continue;
        std::string filename = entry.path().filename().string();

        // all mr-out-* files
        if (filename.find("mr-out-") != 0) continue;

        spdlog::info("filename: {}", filename);

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
