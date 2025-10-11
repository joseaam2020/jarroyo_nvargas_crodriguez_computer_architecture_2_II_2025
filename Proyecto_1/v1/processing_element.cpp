#include "processing_element.h"
#include "interconnect.h"
#include "snoop.h"

ProcessingElement::ProcessingElement(int id, Interconnect *bus) : pe_id(id) {
  this->cache = new Cache(id);
  this->snoop = new SnoopModule(id, this->cache, bus);
  this->cache->setSnoop(this->snoop);
}

ProcessingElement::~ProcessingElement() {}

void ProcessingElement::load(short reg, short regd) {
  // Dato en cache
  if (isValidRegister(reg) & isValidRegister(regd)) {
    double data = this->cache->getData(this->regs[regd]);
    this->regs[reg] = data;
  }
}

void ProcessingElement::store(short reg, short regd) {
  // Guardar dato en cache
  if (isValidRegister(reg) & isValidRegister(regd)) {
    double value = this->regs[reg];
    this->cache->setData(this->regs[regd], value);
  }
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

void ProcessingElement::mov(short reg, double value) {
  if (isValidRegister(reg)) {
    this->regs[reg] = value;
  }
}

void ProcessingElement::printStatus() const {
  std::cout << "============================\n";
  std::cout << "Estado del Processing Element (PE " << pe_id << ")\n";
  std::cout << "----------------------------\n";
  std::cout << "Registros:\n";

  for (int i = 0; i < NUMERO_REGISTROS; ++i) {
    std::cout << "  R" << i << ": " << std::fixed << std::setprecision(4)
              << regs[i] << "\n";
  }

  std::cout << "----------------------------\n";
  std::cout << "Estado de la Cache:\n";

  if (cache) {
    cache->printCache();
  } else {
    std::cout << "  (Cache no asignada)\n";
  }

  std::cout << "============================\n";
}

bool ProcessingElement::isValidRegister(short reg) const {
  if (reg < 0 || reg >= NUMERO_REGISTROS) {
    std::cerr << "Registro inválido: " << reg << std::endl;
    return false;
  }
  return true;
}
