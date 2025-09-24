#include "mapreduce.h"
/*
The Map invocations are distributed across multiple machines by automatically
partitioning the input data into a set of M splits.
The input splits can be processed in parallel by different machines.
Reduce invocations are distributed across multiple machines by partitioning
the intermediate key space into R regions using the user-defined partitioning function.
*/
//integer hash function
//determines which reduce task a key belongs to
int ihash(const std::string &key, int nReduce) {
    //hash function object in STL,string to size_t
    std::hash<std::string> h;
    return static_cast<int>(h(key) % nReduce);
}
