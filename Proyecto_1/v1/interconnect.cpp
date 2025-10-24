#include "interconnect.h"
#include "snoop.h"
#include <iostream>

Interconnect::Interconnect(Memory *mem) : memory(mem) {}

// Recibo los snoops
void Interconnect::registerSnoopModule(SnoopModule *snoop) {
  snoop_modules.push_back(snoop);
}

// Señal de lectura desde el snoop
Interconnect::BusResult Interconnect::broadcastRead(int requesting_pe,
                                                    int address) {
  std::unique_lock<std::mutex> lock(bus_mutex); // Esperar turno del bus
  std::cout << "\n[BUS] PE" << requesting_pe << " broadcasts READ for address "
            << address << std::endl;

  BusResult result;

  // Reviso cada snoop, me devuelve el estado de cada uno.
  for (auto snoop : snoop_modules) {

    // No preguntar al mismo PE que está haciendo el request
    if (snoop->getPEId() == requesting_pe) {
      continue;
    }

    auto response = snoop->handleBusRead(address);

    // Si es un hit
    if (response.hit) {
      result.shared = true;
      // Copiar los datos del bloque completo
      for (int i = 0; i < 4; i++) {
        result.data[i] = response.data[i];
      }
    }
  }

  std::array<double, 4> block; // Para retornar el bloque

  // Si nadie lo tiene voy a memoria a leer el dato
  if (!result.modified && !result.shared) {
    std::cout << "  [BUS] No cache hit, fetching from memory" << std::endl;
    block = memory->read(address);
    // Copiar datos de memoria al resultado

    for (int i = 0; i < 4; i++) {
      result.data[i] = block[i];
      /*
    std::cout << "Dato " << i << " Recibido: result = " << result.data[i]
              << ", block =" << block[i] << std::endl;
    */
    }
  }

  return result; // El bloque se libera de forma automática cuando sale
}

// Notifico a las snoops en ponerse en invalidate
void Interconnect::broadcastInvalidate(int requesting_pe, int address) {
  std::unique_lock<std::mutex> lock(bus_mutex); // Esperar turno del bus
  std::cout << "\n[BUS] PE" << requesting_pe
            << " broadcasts INVALIDATE for address " << address << std::endl;

  for (auto snoop : snoop_modules) {
    // Saltar el PE que está solicitando sin invalidarse a sí mismo
    if (snoop->getPEId() == requesting_pe) {
      continue;
    }

    snoop->handleBusInvalidate(address);
  }
}

void Interconnect::printBusActivity(const std::string &message) {
  std::cout << "[BUS] " << message << std::endl;
}

void Interconnect::writeToMem(int address, double value) {
  memory->write(address, value);
}
