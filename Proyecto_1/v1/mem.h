#ifndef MEM_H
#define MEM_H

#include <array>
#include <cstdint> // para uint64_t
#include <iomanip> // para std::setw y std::setprecision
#include <iostream>
#include <memory> // para smart pointers, opcional
#include <stdexcept>
#include <vector>

// Declaración de la clase
class Memory {
private:
  std::vector<double> storage; // Vector para almacenar datos (memoria)
  int access_count;

public:
  Memory(); // Constructor de la clase

  // Leer un dato de la memoria dada una dirección
  std::array<double, 4> read(int address);

  // Escribir un dato en la memoria dada dirección y el dato
  void write(int address, double data);

  // Inicializa una dirección de memoria con un dato específico
  void initialize(int address, double data);

  // Verifica que la dirección esté en el rango correcto
  int error(int address);

  // Imprime todas las direcciones y sus datos almacenados
  void printMemory() const;

  // Devuelve el número de veces que se ha accedido a la memoria
  int getAccessCount() const { return access_count; }
};

#endif
