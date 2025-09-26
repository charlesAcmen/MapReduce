#pragma once
#include <string>
#include <vector>
#include <utility>

// 类型定义
using KeyValue = std::pair<std::string, std::string>;

// The mapping function is called once for each piece of the input.
// In this framework, the key is the name of the file that is being processed,
// and the value is the file's contents. The return value should be a slice of
// key/value pairs, each represented by a KeyValue.
//user-defined map/reduce function interface
//function pointer type
using MapFunc = std::vector<KeyValue>(*)(const std::string &key, const std::string &value);
using ReduceFunc = std::string(*)(const std::string &key, const std::vector<std::string> &values);

int ihash(const std::string &key, int nReduce);
