#ifndef PROCESSING_ELEMENT_H
#define PROCESSING_ELEMENT_H

#include <cstdint>
#define NUMERO_REGISTROS 8

class ProcessingElement {
private:
  int pe_id;
  double regs[NUMERO_REGISTROS] = {};

  bool isValidRegister(short reg) const;

public:
  ProcessingElement(int id);
  ~ProcessingElement();

  void load(short reg, int dir);
  void store(short reg, int dir);
  void fmul(short regd, short ra, short rb);
  void fadd(short regd, short ra, short rb);
  void inc(short reg);
  void dec(short reg);
  void jnz(char *label);

  int getPEId() const { return pe_id; }

  void printStatus() const;
};

#endif
