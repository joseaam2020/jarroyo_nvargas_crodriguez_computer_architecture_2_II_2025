#include "processing_element.h"
#include "interconnect.h"
#include "snoop.h"

ProcessingElement::ProcessingElement(int id, Interconnect *bus) : pe_id(id) {
  this->cache = new Cache(id);
  this->snoop = new SnoopModule(id, this->cache, bus);
  this->cache->setSnoop(this->snoop);
}

ProcessingElement::~ProcessingElement() {}

bool ProcessingElement::isActive() const{
  return this->active;
}

void ProcessingElement::end(){
  if (this->cache){
    std::cout<<"[PE" << pe_id<<"] haciendo flush" <<std::endl;
    this->cache->flush();
  }

  this->active = false; // Marcar el PE como inactivo
  std::cout<<"[PE" << pe_id<<"] terminó su ejecución" <<std::endl;

}

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

void ProcessingElement::jnz(std::string label) {
  // Modificar el pc para ejecutar la instrucciones
}

void ProcessingElement::mov(short reg, double value) {
  if (isValidRegister(reg)) {
    this->regs[reg] = value;
  }
}

void ProcessingElement::execute(std::string op) {
 
  // Lista para guardar los tokens
  std::vector<std::string> token_list;

  // Descomponer la instrucción en tokens
  std::istringstream ss(op);
  std::string temp_token; // Guarda cada token temporalemte

  while (ss >> temp_token) {

    // Quitar las comas
    if (!temp_token.empty() && temp_token.back() == ',') {
      temp_token.pop_back();
    }

    // Guardar el token en la lista
    token_list.push_back(temp_token);
  }

  // Identificar la operación
  if (!token_list.empty()) {

    // Obtener el primer token
    std::string command = token_list[0];

    if (command == "end") {
      // end
      this->end();
      return; // Salir de la ejecución
    }
    
    
    if (command == "load") {
      // load r5, [r0]
      std::string dest_reg = token_list[1];  // Obtiene r#
      std::string dest_regd = token_list[2]; // Obtiene [r#]
    

      // Obtener el número en r#
      short reg = std::stoi(dest_reg.substr(1)); // r# -> #
    

      short regd = std::stoi(dest_regd.substr(2,3)); // [r#] -> #

      this->load(reg, regd);
      
    }

    if (command == "store") {
      // store r3, [r0]
      std::string dest_reg = token_list[1];  // Obtiene r#
      std::string dest_regd = token_list[2]; // Obtiene [r#]

      // Obtener el número en r#
      short reg = std::stoi(dest_reg.substr(1)); // r# -> #

      // Obtener número de [r#]
      short regd = std::stoi(dest_regd.substr(2,3)); // [r#] -> #
      this->store(reg, regd);
      
    }

    if (command == "fmul") {
      // fmul r3, r1, r2
      std::string dest_regd = token_list[1]; // Obtiene r#
      std::string dest_ra = token_list[2];   // Obtiene r#
      std::string dest_rb = token_list[3];   // Obtiene r#

      // Obtener el número en r#
      short regd = std::stoi(dest_regd.substr(1)); // r# -> #
      short ra = std::stoi(dest_ra.substr(1));     // r# -> #
      short rb = std::stoi(dest_rb.substr(1));     // r# -> #

      this->fmul(regd, ra, rb);
    }

    if (command == "fadd") {
      // fadd r6, r5, r4
      std::string dest_regd = token_list[1]; // Obtiene r#
      std::string dest_ra = token_list[2];   // Obtiene r#
      std::string dest_rb = token_list[3];   // Obtiene r#

      // Obtener el número en r#
      short regd = std::stoi(dest_regd.substr(1)); // r# -> #
      short ra = std::stoi(dest_ra.substr(1));     // r# -> #
      short rb = std::stoi(dest_rb.substr(1));     // r# -> #

      this->fadd(regd, ra, rb);
    }

    if (command == "inc") {
      // inc r1
      std::string dest_reg = token_list[1]; // Obtiene r#

      // Obtener el número en r#
      short reg = std::stoi(dest_reg.substr(1)); // r# -> #

      this->inc(reg);
    }

    if (command == "dec") {
      // dec r5
      std::string dest_reg = token_list[1]; // Obtiene r#

      // Obtener el número en r#
      short reg = std::stoi(dest_reg.substr(1)); // r# -> #

      this->dec(reg);
    }

    if (command == "jnz") {
      // jnz loop
      std::string label = token_list[1]; // Obtiene label

      this->jnz(label);
    }

    if (command == "mov") {
      // mov  r2, #3
      std::string dest_reg = token_list[1];   // Obtiene r#
      std::string dest_value = token_list[2]; // Obtiene #num

      // Obtener el número en r#
      short reg = std::stoi(dest_reg.substr(1));     // r# -> #
      short value = std::stoi(dest_value.substr(1)); // #num -> num

      this->mov(reg, value);
    }
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
