#include "cache.h"
#include "mesi_state.h"
#include <cstdio>
#include <iostream>
#include <ostream>

Cache::Cache(int id) : pe_id(id) {
  sets.resize(num_sets, std::vector<CacheLine>(num_ways));
}

double Cache::getData(int addr) {

  // 1. Verificar alineación
  if (addr % 8 != 0) {
    throw std::runtime_error("Error: 'addr' no es múltiplo de 8.");

    

  }
  stats.reads++;
  CacheLine *line = this->getLine(addr); // Buscar en Cache

  if (line &&
      line->state != mesi_state::INVALID) { // Si se encuentra retornar valor en
    stats.cache_hits++;                                        // cache y es valida
    int word_index = addr / 8;
    int offset = word_index % 4; // posición en el bloque (0 a 3)
    return line->data[offset];
  } else { // Sino

    stats.cache_misses++;
    stats.bus_traffic++;


    int word_index = addr / 8;
    int offset = word_index % 4; // posición en el bloque (0 a 3)
    int block_number = word_index / 4;
    int index = block_number % 8; // set index (0 a 7)
    int tag = addr >> 8;          // parte alta de la dirección

    // 4. MISS: seleccionar vía menos usada (LFU)
    int lfu_way = 0;
    int min_usage = sets[index][0].usage_count;
    for (int way = 1; way < num_ways; ++way) {
      if (sets[index][way].usage_count < min_usage) {
        min_usage = sets[index][way].usage_count;
        lfu_way = way;
      }
    }

    // Linea a cambiar
    CacheLine old_line = sets[index][lfu_way];

    // Pedirle a Snoop que solicite a Interconnect linea de memoria
    CacheLine new_line = this->snoop->handleReadMiss(addr);
    /*
    for (int i = 0; i < 4; i++) {
      std::cout << "Dato " << i << " Recibido : " << new_line.data[i]
                << std::endl;
    }*/

    if (line) {
      line->state = new_line.state;
      line->usage_count++;
      for (short i = 0; i < 4; i++) {
        line->data[i] = new_line.data[i];
      }
    } else {
      new_line.tag = tag;
      new_line.usage_count = 1;

      sets[index][lfu_way] = new_line;

      // Actualizar memoria si tag diferente de -1 y linea valida;
      int base_index = word_index - offset;
      if (old_line.tag != -1 && old_line.state != mesi_state::INVALID) {
        for (short i = 0; i < 4; i++) {
          snoop->writeToMem((base_index + i) * 8, old_line.data[i]);
        }
      }
    }
    return new_line.data[offset];
  }
}

void Cache::setData(int addr, double value) {
  // 1. Verificar alineación
  if (addr % 8 != 0) {
    throw std::runtime_error("Error: 'addr' no es múltiplo de 8.");

    
  }

    stats.writes++;
  CacheLine *verified_line = this->getLine(addr);
  CacheLine line = this->snoop->handleWrite(addr, value);

  // printf("ESTADO LINEA: %s", mesiStateToString(line.state));

  if (!(line.state == mesi_state::INVALID)) { // Si la linea no es invalida
    stats.cache_misses++;
   
    int word_index = addr / 8;
    int offset = word_index % 4; // posición en el bloque (0 a 3)
    int block_number = word_index / 4;
    int index = block_number % 8; // set index (0 a 7)
    int tag = addr >> 8;          // parte alta de la dirección

    // 4. MISS: seleccionar vía menos usada (LFU)
    int lfu_way = 0;
    int min_usage = sets[index][0].usage_count;
    for (int way = 1; way < num_ways; ++way) {
      if (sets[index][way].usage_count < min_usage) {
        min_usage = sets[index][way].usage_count;
        lfu_way = way;
      }
    }

    // Linea a cambiar
    CacheLine old_line = sets[index][lfu_way];

    if (verified_line) {
      if (verified_line->state == mesi_state::INVALID) {
        
        verified_line->state = line.state;
        verified_line->usage_count++;
        for (short i = 0; i < 4; i++) {
          verified_line->data[i] = line.data[i];
        }
      }

    } else {
      line.tag = tag;
      line.usage_count = 1;

      sets[index][lfu_way] = line;
      

      // Actualizar memoria si tag diferente de -1 y linea valida;
      int base_index = word_index - offset;
      if (old_line.tag != -1 && old_line.state != mesi_state::INVALID) {
       
        for (short i = 0; i < 4; i++) {
          snoop->writeToMem((base_index + i) * 8, old_line.data[i]);
        }
      }
    }
  }

  // Nota: en caso de que la linea sea invalida, el Snoop actualiza el valor de
  // la cache
  stats.cache_hits++;
}

void Cache::printCache() const {
  std::cout << "Cache (PE " << pe_id << "):\n";

  for (int set = 0; set < num_sets; ++set) {
    std::cout << "Set " << set << ":\n";

    for (int way = 0; way < num_ways; ++way) {
      const CacheLine &line = sets[set][way];

      std::cout << "  Way " << way << " | Tag: " << std::setw(4) << line.tag
                << " | Usage: " << std::setw(3) << line.usage_count
                << " | MesiState: " << mesiStateToString(line.state)
                << " | Data: [";

      for (int i = 0; i < 4; ++i) {
        std::cout << std::fixed << std::setprecision(2) << line.data[i];
        if (i < 3)
          std::cout << ", ";
      }

      std::cout << "]\n";
    }
  }
}

CacheLine *Cache::getLine(int address) {
  // 1. Verificar alineación
  if (address % 8 != 0) {
    throw std::runtime_error("Error: 'addr' no es múltiplo de 8.");
  }

  // 2. Calcular campos de la dirección
  int word_index = address / 8;
  int offset = word_index % 4; // posición en el bloque (0 a 3)
  int block_number = word_index / 4;
  int index = block_number % 8; // set index (0 a 7)
  int tag = address >> 8;       // parte alta de la dirección

  for (int way = 0; way < num_ways; ++way) {
    CacheLine &line = sets[index][way];
    if (line.tag == tag) {
      return &line;
    }
  }
  return nullptr;
}

void Cache::setSnoop(SnoopModule *snoop) { this->snoop = snoop; }

void Cache::flush() {
  for (short current_set = 0; current_set < num_sets; current_set++) {
    auto &set = sets[current_set];
    for (short current_way = 0; current_way < num_ways; current_way++) {
      const CacheLine &line = set[current_way];

      if (line.state != mesi_state::INVALID) {
        stats.bus_traffic++;
        short block_number = (line.tag << 3) | current_set;

        for (short offset = 0; offset < 4; offset++) {
          short word_index = block_number * 4 + offset;
          int address = word_index * 8; // Usa int por seguridad

          snoop->writeToMem(address, line.data[offset]);
        }
      }
    }
  }
}
