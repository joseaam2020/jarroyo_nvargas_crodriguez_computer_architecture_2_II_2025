#include "cache.h"
#include "mesi_state.h"
#include <ostream>

Cache::Cache(int id, SnoopModule *snoop) : pe_id(id), snoop(snoop) {
  sets.resize(num_sets, std::vector<CacheLine>(num_ways));
}

double Cache::getData(int addr) {

  // 1. Verificar alineación
  if (addr % 8 != 0) {
    throw std::runtime_error("Error: 'addr' no es múltiplo de 8.");
  }

  CacheLine *line = this->getLine(addr); // Buscar en Cache

  if (line &&
      line->state != mesi_state::INVALID) { // Si se encuentra retornar valor en
                                            // cache y es valida
    int word_index = addr / 8;
    int offset = word_index % 4; // posición en el bloque (0 a 3)
    return line->data[offset];
  } else { // Sino
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

    CacheLine old_line = sets[index][lfu_way];

    // Actualizar memoria si tag diferente de -1;
    int base_index = word_index - offset;
    if (tag != -1 && old_line.state != mesi_state::INVALID) {
      for (short i = 0; i < 4; i++) {
        snoop->writeToMem((base_index + i) * 8, old_line.data[i]);
      }
    }

    CacheLine new_line = this->snoop->handleReadMiss(addr);
    new_line.tag = tag;
    new_line.usage_count = 1;

    sets[index][lfu_way] = new_line;

    return new_line.data[offset];
  }
}

void Cache::setData(int addr, double value) {
  // 1. Verificar alineación
  if (addr % 8 != 0) {
    throw std::runtime_error("Error: 'addr' no es múltiplo de 8.");
  }

  CacheLine line = this->snoop->handleWrite(addr, value);

  if (!(line.state == mesi_state::INVALID)) {
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

    CacheLine old_line = sets[index][lfu_way];

    // Actualizar memoria si tag diferente de 1;
    int base_index = word_index - offset;
    if (tag != -1 && old_line.state != mesi_state::INVALID) {
      for (short i = 0; i < 4; i++) {
        snoop->writeToMem((base_index + i) * 8, old_line.data[i]);
      }
    }

    line.tag = tag;
    line.usage_count = 1;

    sets[index][lfu_way] = line;
  }
}

void Cache::printCache() const {
  std::cout << "Cache (PE " << pe_id << "):\n";

  for (int set = 0; set < num_sets; ++set) {
    std::cout << "Set " << set << ":\n";

    for (int way = 0; way < num_ways; ++way) {
      const CacheLine &line = sets[set][way];

      std::cout << "  Way " << way << " | Tag: " << std::setw(4) << line.tag
                << " | Usage: " << std::setw(3) << line.usage_count
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
