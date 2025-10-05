#include "cache.h"

Cache::Cache(int id) : pe_id(id) {}

bool Cache::hasLine(int address) const {
    return cache_lines.find(address) != cache_lines.end() &&
           cache_lines.at(address).state != MESIState::INVALID;
}

CacheLine& Cache::getLine(int address) {
    return cache_lines[address];
}

MESIState Cache::getState(int address) const {
    auto it = cache_lines.find(address);
    if (it != cache_lines.end()) {
        return it->second.state;
    }
    return MESIState::INVALID;
}

int Cache::getData(int address) const {
    auto it = cache_lines.find(address);
    if (it != cache_lines.end()) {
        return it->second.data;
    }
    return 0;
}

void Cache::setState(int address, MESIState state) {
    if (cache_lines.find(address) != cache_lines.end()) {
        cache_lines[address].state = state;
    }
}

void Cache::setData(int address, int data) {
    if (cache_lines.find(address) != cache_lines.end()) {
        cache_lines[address].data = data;
    }
}

void Cache::insertLine(int address, int data, MESIState state) {
    cache_lines[address] = CacheLine(address, data, state);
}

void Cache::invalidateLine(int address) {
    if (cache_lines.find(address) != cache_lines.end()) {
        cache_lines[address].state = MESIState::INVALID;
    }
}

void Cache::printCache() const {
    std::cout << "PE" << pe_id << " Cache:" << std::endl;
    for (const auto& pair : cache_lines) {
        if (pair.second.state != MESIState::INVALID) {
            std::cout << "  Addr: " << pair.first
                      << " | Data: " << pair.second.data
                      << " | State: " << stateToString(pair.second.state)
                      << std::endl;
        }
    }
}
