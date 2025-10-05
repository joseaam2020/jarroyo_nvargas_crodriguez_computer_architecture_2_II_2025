#ifndef CACHE_H
#define CACHE_H

#include "mesi_protocol.h"
#include <map>

struct CacheLine {
    int address;
    int data;
    MESIState state;

    CacheLine() : address(-1), data(0), state(MESIState::INVALID) {}
    CacheLine(int addr, int d, MESIState s) : address(addr), data(d), state(s) {}
};

class Cache {
private:
    int pe_id;
    std::map<int, CacheLine> cache_lines;

public:
    Cache(int id);

    bool hasLine(int address) const;
    CacheLine& getLine(int address);
    MESIState getState(int address) const;
    int getData(int address) const;

    void setState(int address, MESIState state);
    void setData(int address, int data);
    void insertLine(int address, int data, MESIState state);
    void invalidateLine(int address);

    void printCache() const;
    int getPEId() const { return pe_id; }
};

#endif
