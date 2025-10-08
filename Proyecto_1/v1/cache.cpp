#include "cache.h"
#include <cmath>
#include <stdexcept>

Cache::Cache(int id) : pe_id(id) {
  sets.resize(num_sets, std::vector<CacheLine>(num_ways));
}

void Cache::setData(int addr, double value) {
  // Cálculos de campos
  int word_index;
  if (addr % 8 == 0) {
    word_index = addr / 8;
  } else {
    throw std::runtime_error("Error: 'addr' no es múltiplo de 8.");
  }

  int offset = word_index % 4;
  int block_number = word_index / 4;
  int index = block_number % 8;
  int tag = addr >> 8;

  // Buscar si el bloque ya está en caché (HIT)
  for (int way = 0; way < num_ways; ++way) {
    CacheLine &line = sets[index][way];
    if (line.tag == tag) {
      line.data[offset] = value;
      return;
    }
  }

  // MISS: reemplazar la primera vía (podrías usar LRU más adelante)
  CacheLine &replacement = sets[index][0];
  replacement = CacheLine(tag);
  replacement.data[offset] = value;

  // replacement.state = MESIState::MODIFIED;  // Si lo usas en el futuro
}
