#ifndef MEMORY_H
#define MEMORY_H

#include <map>
#include <iostream>

class Memory {
private:
    std::map<int, int> storage;
    int access_count;

public:
    Memory();

    int read(int address);
    void write(int address, int data);

    void initialize(int address, int data);
    void printMemory() const;
    int getAccessCount() const { return access_count; }
};

#endif
