#ifndef MESI_PROTOCOL_H
#define MESI_PROTOCOL_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <memory>

enum class MESIState {
    MODIFIED,   // M: Cache line modified, dirty, exclusive to this cache
    EXCLUSIVE,  // E: Cache line clean, exclusive to this cache
    SHARED,     // S: Cache line clean, shared with other caches
    INVALID     // I: Cache line invalid
};

enum class BusTransaction {
    READ,           // Read request
    WRITE,          // Write request
    READ_X,         // Read with intent to modify
    INVALIDATE,     // Invalidate other copies
    FLUSH,          // Write back to memory
    NONE
};

struct BusMessage {
    int pe_id;
    BusTransaction transaction;
    int address;
    int data;

    BusMessage(int id, BusTransaction trans, int addr, int d = 0)
        : pe_id(id), transaction(trans), address(addr), data(d) {}
};

std::string stateToString(MESIState state);
std::string transactionToString(BusTransaction trans);

#endif
