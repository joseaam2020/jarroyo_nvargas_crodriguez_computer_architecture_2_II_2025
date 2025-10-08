#ifndef CACHE_H
#define CACHE_H

// #include "mesi_protocol.h"
#include <vector>

struct CacheLine {
  int tag = -1;        // Identificador del bloque de memoria
  double data[4] = {}; // Cada línea almacena 4 palabras (32 bytes)
  // MESIState state;  // Preparado para coherencia

  CacheLine() = default;
  CacheLine(int tag) : tag(tag) {}
};

class Cache {
private:
  int pe_id;

  static constexpr int num_sets = 8; // 16 bloques / 2 vías = 8 conjuntos
  static constexpr int num_ways = 2;

  std::vector<std::vector<CacheLine>> sets;

public:
  Cache(int id);

  bool hasLine(int address) const;
  CacheLine &getLine(int address);
  // MESIState getState(int address) const;
  double getData(int address, int offset) const;

  // void setState(int address, MESIState state);
  void setData(int addr, double value);
  // void insertLine(int address, int data, MESIState state);
  void invalidateLine(int address);

  void printCache() const;
  int getPEId() const { return pe_id; }
};

#endif
