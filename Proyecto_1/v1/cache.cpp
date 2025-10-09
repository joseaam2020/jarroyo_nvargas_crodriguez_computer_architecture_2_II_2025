#include "cache.h"

Cache::Cache(int id) : pe_id(id) {
  sets.resize(num_sets, std::vector<CacheLine>(num_ways));
}

double Cache::getData(int addr) {
  // 1. Verificar alineación
  if (addr % 8 != 0) {
    throw std::runtime_error("Error: 'addr' no es múltiplo de 8.");
  }

  // 2. Calcular campos de la dirección
  int word_index = addr / 8;
  int offset = word_index % 4; // posición en el bloque (0 a 3)
  int block_number = word_index / 4;
  int index = block_number % 8; // set index (0 a 7)
  int tag = addr >> 8;          // parte alta de la dirección

  // 3. Buscar si el bloque está en la caché (HIT)
  for (int way = 0; way < num_ways; ++way) {
    CacheLine &line = sets[index][way];
    if (line.tag == tag) {
      // ‼️ IMPORTANTE hay que revisar que sea valido
      line.usage_count++;       // aumentar frecuencia de uso
      return line.data[offset]; // devolver valor almacenado
    }
  }

  // 4. MISS: seleccionar vía menos usada (LFU)
  int lfu_way = 0;
  int min_usage = sets[index][0].usage_count;
  for (int way = 1; way < num_ways; ++way) {
    if (sets[index][way].usage_count < min_usage) {
      min_usage = sets[index][way].usage_count;
      lfu_way = way;
    }
  }

  // 5. Reemplazar el bloque en la vía seleccionada
  CacheLine &replacement = sets[index][lfu_way];
  replacement.tag = tag;
  replacement.usage_count = 1;

  // 6. Broadcast a interconnect

  // 7. Devolver la palabra solicitada
  return replacement.data[offset];
}

void Cache::setData(int addr, double value) {
  // 1. Verificar alineación
  if (addr % 8 != 0) {
    throw std::runtime_error("Error: 'addr' no es múltiplo de 8.");
  }

  // 2. Calcular campos de la dirección
  int word_index = addr / 8;
  int offset = word_index % 4; // posición en el bloque (0 a 3)
  int block_number = word_index / 4;
  int index = block_number % 8; // set index (0 a 7)
  int tag = addr >> 8;          // parte alta de la dirección

  // 3. Buscar si el bloque ya está en caché (HIT)
  for (int way = 0; way < num_ways; ++way) {
    CacheLine &line = sets[index][way];
    if (line.tag == tag) {
      // ‼️ IMPORTANTE hay que revisar que sea valido
      line.data[offset] = value;
      line.usage_count++;
      return;
    }
  }

  // 4. MISS: reemplazo LFU
  int min_usage = sets[index][0].usage_count;
  int lfu_way = 0;

  for (int way = 1; way < num_ways; ++way) {
    if (sets[index][way].usage_count < min_usage) {
      min_usage = sets[index][way].usage_count;
      lfu_way = way;
    }
  }

  // 5. Reemplazar solo lo necesario
  CacheLine &replacement = sets[index][lfu_way];
  replacement.tag = tag;            // Cambiar el tag
  replacement.data[offset] = value; // Escribir solo la palabra modificada
  replacement.usage_count = 1;      // Reiniciar contador (opcional)

  // 6. Broadcast a interconnect

  // replacement.state = MESIState::MODIFIED;  // Si lo usas en el futuro
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
