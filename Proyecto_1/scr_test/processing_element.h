#ifndef PROCESSING_ELEMENT_H
#define PROCESSING_ELEMENT_H

#include "cache.h"
#include "snoop.h"
#include "interconnect.h"

class ProcessingElement {
private:
    int pe_id;
    Cache* cache;
    SnoopModule* snoop;
    Interconnect* bus;

public:
    ProcessingElement(int id, Interconnect* interconnect);
    ~ProcessingElement();

    void read(int address);
    void write(int address, int data);

    Cache* getCache() { return cache; }
    SnoopModule* getSnoop() { return snoop; }
    int getPEId() const { return pe_id; }

    void printStatus() const;
};

#endif
