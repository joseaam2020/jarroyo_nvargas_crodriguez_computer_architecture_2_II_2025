#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>
#include <cstdint>  // para uint64_t
#include <memory>   // para smart pointers, opcional
#include <vector>


// Declaración de la clase
class Memory {
private:
    std::vector<double> storage; // Vector para almacenar datos (memoria)
    int access_count;

public:
    Memory(); // Constructor de la clase

    // Leer un dato de la memoria dada una dirección
    int read(int address);

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
