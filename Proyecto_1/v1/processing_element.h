#ifndef PROCESSING_ELEMENT_H
#define PROCESSING_ELEMENT_H

#include "cache.h"
#include <cstdint>
#include <iomanip> // Para formatear decimales
#include <iostream>
#define NUMERO_REGISTROS 8

class ProcessingElement {
private:
  int pe_id;
  double regs[NUMERO_REGISTROS] = {};
  Cache *cache;

  bool isValidRegister(short reg) const;

public:
  ProcessingElement(int id);
  ~ProcessingElement();

  void load(short reg, short regd);
  void store(short reg, short regd);
  void fmul(short regd, short ra, short rb);
  void fadd(short regd, short ra, short rb);
  void inc(short reg);
  void dec(short reg);
  void jnz(char *label);
  void mov(short reg, double value);

  int getPEId() const { return pe_id; }

  void printStatus() const;
};

#endif
