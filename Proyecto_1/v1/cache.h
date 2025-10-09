#ifndef CACHE_H
#define CACHE_H

// #include "mesi_protocol.h"
#include <cmath>
#include <iomanip> // Para formatear la salida con precisión
#include <iostream>
#include <stdexcept>
#include <vector>

struct CacheLine {
  int tag = -1;        // Identificador del bloque de memoria
  double data[4] = {}; // Cada línea almacena 4 palabras (32 bytes)
  int usage_count = 0;

  // MESIState state;  // Preparado para coherencia

  CacheLine() = default;
  CacheLine(int tag) : tag(tag), usage_count(1) {}
};

class Cache {
private:
  int pe_id;

  static constexpr int num_sets = 8; // 16 bloques / 2 vías = 8 conjuntos
  static constexpr int num_ways = 2;

  std::vector<std::vector<CacheLine>> sets;

public:
  Cache(int id);

  /*
  puede que no sea necesario?
  bool hasLine(int address) const;
  CacheLine &getLine(int address);
  void insertLine(int address, int data, MESIState state);
   */

  //  MESIState getState(int address) const;
  // void setState(int address, MESIState state);

  double getData(int addr);
  void setData(int addr, double value);

  void invalidateLine(int address);

  int getPEId() const { return pe_id; }
  void printCache() const;
};

#endif
