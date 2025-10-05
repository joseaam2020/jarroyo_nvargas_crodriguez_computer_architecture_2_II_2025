#ifndef SNOOP_H
#define SNOOP_H

#include "mesi_protocol.h"
#include "cache.h"

class SnoopModule {
private:
    Cache* cache;
    int pe_id;

public:
    SnoopModule(int id, Cache* c);

    struct SnoopResponse {
        bool hit;
        bool shared;
        bool modified;
        int data;

        SnoopResponse() : hit(false), shared(false), modified(false), data(0) {}
    };

    SnoopResponse handleBusRead(int address);
    SnoopResponse handleBusReadX(int address);
    void handleBusInvalidate(int address);

    void processLocalRead(int address, bool shared_signal, int data_from_source);
    void processLocalWrite(int address, int data);

private:
    void transitionOnRead(int address, bool other_caches_have);
    void transitionOnWrite(int address);
};

#endif
