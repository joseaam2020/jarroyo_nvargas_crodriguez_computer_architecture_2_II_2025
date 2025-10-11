#ifndef CACHE_H
#define CACHE_H

// #include "mesi_protocol.h"
#include "mesi_state.h"
#include "snoop.h"
#include <cmath>
#include <iomanip> // Para formatear la salida con precisión
#include <iostream>
#include <stdexcept>
#include <vector>

class SnoopModule;

struct CacheLine {
  int tag = -1;        // Identificador del bloque de memoria
  double data[4] = {}; // Cada línea almacena 4 palabras (32 bytes)
  int usage_count = 0;

  mesi_state state; // Preparado para coherencia

  CacheLine() = default;
  CacheLine(int tag) : tag(tag), usage_count(1) {}
};

class Cache {
private:
  int pe_id;
  SnoopModule *snoop;

  static constexpr int num_sets = 8; // 16 bloques / 2 vías = 8 conjuntos
  static constexpr int num_ways = 2;

  std::vector<std::vector<CacheLine>> sets;

public:
  Cache(int id, SnoopModule *snoop);

  /*
  puede que no sea necesario?
  CacheLine &getLine(int address);
  void insertLine(int address, int data, MESIState state);
   */

  //  MESIState getState(int address) const;
  // void setState(int address, MESIState state);

  CacheLine *getLine(int address);
  double getData(int addr);
  void setData(int addr, double value);

  int getPEId() const { return pe_id; }
  void printCache() const;
};

#endif
