#ifndef INTERCONNECT_H
#define INTERCONNECT_H

#include "mem.h"
#include <vector>

class SnoopModule;

class Interconnect {
private:
  std::vector<SnoopModule*> snoop_modules;
  Memory *memory;

public:
  Interconnect(Memory *mem);
  
  void registerSnoopModule(SnoopModule* snoop);

  struct BusResult {
    bool shared;
    bool modified;
    double data[4];
    int owner_pe;

    BusResult() : shared(false), modified(false), owner_pe(-1) {
      for (int i = 0; i < 4; i++)
        data[i] = 0.0;
    }
  };

  BusResult broadcastRead(int requesting_pe, int address);
  void broadcastInvalidate(int requesting_pe, int address);
  void writeToMem(int address, double value);

  void printBusActivity(const std::string &message);
};

#endif
