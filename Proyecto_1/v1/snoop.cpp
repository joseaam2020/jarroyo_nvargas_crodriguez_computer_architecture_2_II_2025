#include "snoop.h"
#include "cache.h"
#include "interconnect.h"
#include "mesi_state.h"
#include <iostream>

// Implementación del clase
SnoopModule::SnoopModule(int id, Cache *c, Interconnect *ic)
    : pe_id(id), cache(c), interconnect(ic) {
  std::cout << "[SNOOP " << pe_id << "] Inicializado" << std::endl;
}

// Peticiones externas
// Otro PE quiere leer
SnoopModule::SnoopResponse
SnoopModule::handleBusRead(int address) { // devuelva datos
  // Respuesta
  SnoopResponse response;
  response.hit = true;

  // Obtiene linea de cache
  CacheLine *line = cache->getLine(address);
  if (line) {
    mesi_state *state = &line->state;
    // Manejar transiciones según el estado actual
    switch (*state) {
    case mesi_state::MODIFIED:
      std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in MODIFIED -> "
                << "Flush to memory & transition to SHARED" << std::endl;
      response.modified = true;
      response.shared = true;
      *state = mesi_state::SHARED;
      break;

    case mesi_state::EXCLUSIVE:
      std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in EXCLUSIVE -> "
                << "transition to SHARED" << std::endl;
      response.shared = true;
      *state = mesi_state::SHARED;
      break;

    case mesi_state::SHARED:
      std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in SHARED -> "
                << "remain SHARED" << std::endl;
      response.shared = true;
      break;

    case mesi_state::INVALID:
      response.hit = false;
      break;
    }
  } else {
    response.hit = false;
  }

  if (response.hit) {
    for (short i = 0; i < 4; i++) {
      response.data[i] = line->data[i];
    }
  }

  return response;
}

// Invalidar porque otro PE está escribiendo
void SnoopModule::handleBusInvalidate(int address) {
  // Obtiene linea de cache
  CacheLine *line = cache->getLine(address);
  if (line) {
    mesi_state *state = &line->state;
    std::cout << "  [PE" << pe_id << " Snoop] Invalidate signal -> "
              << "INVALIDATE line" << std::endl;
    *state = mesi_state::INVALID; // la invalida
  }
}

// Parte de Interconnect

/**
 * Read Miss usando el interconnect
 *
 * 1. Solicitar dato al interconnect (broadcastRead)
 * 2. Interconnect pregunta a otros snoops
 * 3. Recibir dato y determinar estado (SHARED o EXCLUSIVE)
 * 4. Insertar en caché local
 */
CacheLine SnoopModule::handleReadMiss(int address) {
  if (!interconnect) {
    std::cerr << "[SNOOP " << pe_id
              << "] ERROR: No hay interconnect configurado" << std::endl;
    return false;
  }

  std::cout << "\n[SNOOP " << pe_id << "] Manejando Read miss para addr 0x"
            << std::hex << address << std::dec << std::endl;

  // Paso 1: Solicitar datos al interconnect
  auto bus_result = interconnect->broadcastRead(pe_id, address);

  // Paso 2: Determinar estado basado en si está compartido
  bool shared_signal = bus_result.shared || bus_result.modified;
  mesi_state new_state =
      shared_signal ? mesi_state::SHARED : mesi_state::EXCLUSIVE;

  CacheLine new_line;
  new_line.state = new_state;
  for (short i = 0; i < 4; i++) {
    new_line.data[i] = bus_result.data[i];
    std::cout << "Dato " << i << " Recibido : " << new_line.data[i]
              << std::endl;
  }

  return new_line;
}

/**
 * Maneja un write (hit o miss) usando el interconnect
 *
 * 1. Si es MISS, traer dato primero
 * 2. Invalidar otras copias (broadcastInvalidate)
 * 3. Escribir dato en caché con estado MODIFIED
 */
CacheLine SnoopModule::handleWrite(int address, double value) {
  if (!interconnect) {
    std::cerr << "[SNOOP " << pe_id
              << "] ERROR: No hay interconnect configurado" << std::endl;
    return false;
  }

  std::cout << "\n[SNOOP " << pe_id << "] Manejando WRITE para addr 0x"
            << std::hex << address << std::dec << std::endl;

  CacheLine *line = cache->getLine(address);
  CacheLine new_line;
  if (!line ||
      line->state == mesi_state::INVALID) { // Si no tengo linea en cache
    std::cout << "[SNOOP" << pe_id << " ]" << "WRITE MISS";

    // Pido linea a Interconnect
    CacheLine new_line = this->handleReadMiss(address);
    int word_index = address / 8;
    int offset = word_index % 4; // posición en el bloque (0 a 3)

    // Actualizo datos de la cache
    new_line.data[offset] = value;
    new_line.state = mesi_state::MODIFIED;

    // Invalido las lineas de las demas caches
    interconnect->broadcastInvalidate(pe_id, address);
  } else { // Si tengo la linea en cache
    std::cout << "[SNOOP" << pe_id << " ]" << "WRITE HIT";

    // Actualizo valores de la linea
    int word_index = address / 8;
    int offset = word_index % 4; // posición en el bloque (0 a 3)

    line->state = mesi_state::MODIFIED;
    line->data[offset] = value;

    // Invalido las lineas de las demas caches
    interconnect->broadcastInvalidate(pe_id, address);

    // Devuelvo linea invalida para decirle a la cache que ya se actualizo el
    // valor
    new_line = CacheLine();
    new_line.state = mesi_state::INVALID;
  }
  return new_line;
}

void SnoopModule::writeToMem(int address, double value) {
  interconnect->writeToMem(address, value);
}
