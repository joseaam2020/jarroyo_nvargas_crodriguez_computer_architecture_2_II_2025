#include "mem.h"
#include <ostream>

// Número de posiciones de la memoria, tamaño fijo de la memoria
#define MEM_SIZE 512

// Inicializar toda la memoria a cero
Memory::Memory() : storage(MEM_SIZE, 0), access_count(0) {}

std::array<double, 4> Memory::read(int address) {
  access_count++;

  // Verificar dirección válida
  if (error(address) == -1) {
    throw std::runtime_error("Al leer en memoria, direccion fuera de rango");
  }

  // Convertir dirección en bytes a índice de palabra
  int word_index = address / 8;

  // Alinear al inicio del bloque (bloque de 4 palabras)
  int block_start = word_index - (word_index % 4);

  /*
  std::cout << "Address " << address << std::endl;
  std::cout << "Word Index " << word_index << std::endl;
  std::cout << "Block start " << block_start << std::endl;
  */

  // Leer las 4 palabras del bloque
  std::array<double, 4> block;
  for (int i = 0; i < 4; i++) {
    block[i] = storage[block_start + i];
    /*
    std::cout << "Dato " << i << " Leido : " << storage[block_start + i]
              << std::endl;
    */
  }

  return block;
}

// Escribir un dato
void Memory::write(int address, double data) {
  access_count++;

  // Verificar que la dirección está dentro del rango
  if (error(address) == -1) {
    throw std::runtime_error(
        "Al escribir en memoria, direccion fuera de rango");
  }

  // Convertir dirección en bytes a índice de palabra
  int word_index = address / 8;

  // Escribir palabra
  storage[word_index] = data;
}

void Memory::initialize(int address, double data) {
  // Verificar que la dirección está dentro del rango
  if (error(address) == -1) {
    throw std::runtime_error(
        "Al escribir en memoria, direccion fuera de rango");
  }

  // Convertir dirección en bytes a índice de palabra
  int word_index = address / 8;

  // Escribir palabra
  storage[word_index] = data;
}

// Imprimir el contenido de memoria y los accesos a memoria
void Memory::printMemory() const {
  std::cout << "\n===== MEMORY CONTENTS =====" << std::endl;
  std::cout << std::fixed << std::setprecision(2); // mostrar con 2 decimales

  // Cada bloque tiene 4 palabras
  for (size_t i = 0; i < storage.size(); i += 4) {
    std::cout << "Block " << std::setw(3) << (i / 4) << " | ";

    for (size_t j = 0; j < 4 && (i + j) < storage.size(); ++j) {
      std::cout << "Addr[" << std::setw(3) << (i + j) << "] = " << std::setw(8)
                << storage[i + j] << "  ";
    }
    std::cout << '\n';
  }

  std::cout << "\nTotal memory accesses: " << access_count << std::endl;
}

// Verificar que la dirección esté en el rango indicado
int Memory::error(int address) {
  // Direccion en el rango de memoria y alineada con las palabras
  if (address < 0 || address >= MEM_SIZE * 8 || address % 8 != 0) {
    std::cout << "Mala direccion: " << address << ", " << (address < 0)
              << std::endl;
    return -1;
  } else {
    return 0;
  }
}

//// Main de Pueba

// int main() {
//   Memory *memory = new Memory();
//   memory->initialize(0, 15.0);
//   memory->initialize(8, 13.9);
//   memory->initialize(16, 41.3);
//   memory->initialize(24, 100.8);
//
//   memory->initialize(32, 30.0);
//   memory->initialize(40, 27.8);
//   memory->initialize(48, 82.6);
//   memory->initialize(56, 201.6);
//
//   auto block = memory->read(0);
//   for (auto value : block) {
//     std::cout << "Value: " << value << std::endl;
//   }
//
//   memory->printMemory();
//   return 0;
// }
