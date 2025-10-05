#ifndef INTERCONNECT_H
#define INTERCONNECT_H

#include "mesi_protocol.h"
#include "snoop.h"
#include "memory.h"
#include <vector>

class Interconnect {
private:
    std::vector<SnoopModule*> snoop_modules;
    Memory* memory;

public:
    Interconnect(Memory* mem);

    void registerSnoopModule(SnoopModule* snoop);

    struct BusResult {
        bool shared;
        bool modified;
        int data;
        int owner_pe;

        BusResult() : shared(false), modified(false), data(0), owner_pe(-1) {}
    };

    BusResult broadcastRead(int requesting_pe, int address);
    BusResult broadcastReadX(int requesting_pe, int address);
    void broadcastInvalidate(int requesting_pe, int address);

    void printBusActivity(const std::string& message);
};

#endif