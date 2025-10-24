#ifndef INTERCONNECT_H
#define INTERCONNECT_H

#include "mem.h"
#include <vector>
#include <queue>
#include <mutex>
#include <array>

class SnoopModule;

struct BusRequest {
    int pe_id;
    std::string op;
    int address;
    int valude;
  };

class Interconnect {
private:
  std::vector<SnoopModule*> snoop_modules;
  Memory *memory;
  
  // Este es el mutex global para el bus
  // Para asegurar que solo un hilo acceda al bus a la vez
  std::mutex bus_mutex; 

public:
  Interconnect(Memory *mem);
  
  void registerSnoopModule(SnoopModule* snoop);

  struct BusResult {
    bool shared;
    bool modified;
    std::array<double, 4> data;
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
