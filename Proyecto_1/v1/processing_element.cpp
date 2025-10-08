#include "processing_element.h"
#include <iostream>

ProcessingElement::ProcessingElement(int id) : pe_id(id) {}

ProcessingElement::~ProcessingElement() {}

void ProcessingElement::load(short reg, int dir) {
  // Dato en cache
  // Si el dato es valido Hit
  // Si el dato no es valido hacer broadcast para recibir de otra cache
  // No esta en cache
  // Hacer broadcast para recibir de otra cache o de memoria
}

void ProcessingElement::store(short reg, int dir) {
  // Guardar dato en cache
  // Hacer broadcast de store para que las demas caches sepan que escribi el
  // dato
}

void ProcessingElement::fmul(short regd, short ra, short rb) {
  // Valor de registros ra * rb
  if (isValidRegister(regd) & isValidRegister(ra) & isValidRegister(rb)) {
    this->regs[regd] = this->regs[ra] * this->regs[rb];
  }
}

void ProcessingElement::fadd(short regd, short ra, short rb) {
  // Valore de registros ra + rb
  if (isValidRegister(regd) & isValidRegister(ra) & isValidRegister(rb)) {
    this->regs[regd] = this->regs[ra] + this->regs[rb];
  }
}

void ProcessingElement::inc(short reg) {
  // Incrementar el valor del registro
  if (isValidRegister(reg)) {
    this->regs[reg]++;
  }
}

void ProcessingElement::dec(short reg) {
  // Decrementar el valor del registro
  if (isValidRegister(reg)) {
    this->regs[reg]--;
  }
}

void ProcessingElement::jnz(char *label) {
  // Modificar el pc para ejecutar la instrucciones
}

void ProcessingElement::printStatus() const {}

bool ProcessingElement::isValidRegister(short reg) const {
  if (reg < 0 || reg >= NUMERO_REGISTROS) {
    std::cerr << "Registro inválido: " << reg << std::endl;
    return false;
  }
  return true;
}
