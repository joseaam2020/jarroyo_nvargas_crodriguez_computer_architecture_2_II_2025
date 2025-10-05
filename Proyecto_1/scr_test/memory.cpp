#include "memory.h"

Memory::Memory() : access_count(0) {}

int Memory::read(int address) {
    access_count++;
    if (storage.find(address) == storage.end()) {
        storage[address] = 0;
    }
    std::cout << "  [MEMORY] Read address " << address << " -> data: " << storage[address] << std::endl;
    return storage[address];
}

void Memory::write(int address, int data) {
    access_count++;
    storage[address] = data;
    std::cout << "  [MEMORY] Write address " << address << " <- data: " << data << std::endl;
}

void Memory::initialize(int address, int data) {
    storage[address] = data;
}

void Memory::printMemory() const {
    std::cout << "\n=== MEMORY CONTENTS ===" << std::endl;
    for (const auto& pair : storage) {
        std::cout << "Address " << pair.first << ": " << pair.second << std::endl;
    }
    std::cout << "Total memory accesses: " << access_count << std::endl;
}
