#include "interconnect.h"
#include <iostream>

Interconnect::Interconnect(Memory *mem) : memory(mem) {}

// Recibo los snoops
void Interconnect::registerSnoopModule(SnoopModule *snoop) {
  snoop_modules.push_back(snoop);
}

// Señal de lectura desde el snoop
Interconnect::BusResult Interconnect::broadcastRead(int requesting_pe,
                                                    int address) {
  std::cout << "\n[BUS] PE" << requesting_pe << " broadcasts READ for address "
            << address << std::endl;

  BusResult result;
  // Reviso cada snoop, me devuelve el estado de cada uno.
  for (auto snoop : snoop_modules) {
    auto response = snoop->handleBusRead(address);
    // Si es un hit
    if (response.hit) {
      result.shared = true;
      result.data = response.data;

      if (response.modified) {
        result.modified = true;
        std::cout << "  [BUS] Cache provides modified data, flushing to memory"
                  << std::endl;
        memory->write(address, response.data);
      }
    }
  }
  // Si nadie lo tiene voy a memoria a leer el dato
  if (!result.modified && !result.shared) {
    std::cout << "  [BUS] No cache hit, fetching from memory" << std::endl;
    result.data = memory->read(address);
  }

  return result;
}
// En el caso de que sea exclusive
Interconnect::BusResult Interconnect::broadcastReadX(int requesting_pe,
                                                     int address) {
  std::cout << "\n[BUS] PE" << requesting_pe
            << " broadcasts READ_X (exclusive) for address " << address
            << std::endl;

  // Espacio para el estado.
  BusResult result;

  for (auto snoop : snoop_modules) {
    auto response = snoop->handleBusReadX(address);

    if (response.hit && response.modified) {
      result.modified = true;
      result.data = response.data;
      std::cout << "  [BUS] Cache provides modified data, flushing to memory"
                << std::endl;
      memory->write(address, response.data);
    } else if (response.hit) {
      result.data = response.data;
    }
  }

  if (!result.modified) {
    std::cout << "  [BUS] Fetching from memory" << std::endl;
    result.data = memory->read(address);
  }

  return result;
}
// Notifico a las snoops en ponerse en invalidate
void Interconnect::broadcastInvalidate(int requesting_pe, int address) {
  std::cout << "\n[BUS] PE" << requesting_pe
            << " broadcasts INVALIDATE for address " << address << std::endl;

  for (auto snoop : snoop_modules) {
    snoop->handleBusInvalidate(address);
  }
}

void Interconnect::printBusActivity(const std::string &message) {
  std::cout << "[BUS] " << message << std::endl;
}
