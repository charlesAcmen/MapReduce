#include <vector>
#include <string>
#include <cctype>// for std::isalpha
#include "mapreduce.h"
#include "worker.h"
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
    return 0;
}
