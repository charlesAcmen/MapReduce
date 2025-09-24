#pragma once
#include <string>
#include <vector>
#include <utility>

// 类型定义
using KeyValue = std::pair<std::string, std::string>;

//user-defined map/reduce function interface
//function pointer type
using MapFunc = std::vector<KeyValue>(*)(const std::string &key, const std::string &value);
using ReduceFunc = std::string(*)(const std::string &key, const std::vector<std::string> &values);

int ihash(const std::string &key, int nReduce);
