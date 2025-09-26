#include <vector>
#include <string>
#include "coordinator.h"
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
    return 0;
}
