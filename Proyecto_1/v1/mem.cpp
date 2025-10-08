#include "mem.h"

// Número de posiciones de la memoria, tamaño fijo de la memoria
#define MEM_SIZE 512  

// Inicializar toda la memoria a cero
Memory::Memory() : storage(MEM_SIZE, 0), access_count(0) {}
    

// Leer un dato
int Memory::read(int address) {
    access_count++;
    
    // Verificar que la dirección está dentro del rango
    int err = Memory::error(address);
    if (err == -1){
        std::cerr << "Error: direccion fuera del rango \n";
        return err;
    }else {
        // Retorna el valor en la dirección
        return storage[address]; // acceso directo
    }
}    

// Escribir un dato
void Memory::write(int address, double data) {
    access_count++;
    
    // Verificar que la dirección está dentro del rango
    int err = Memory::error(address);
    if (err == -1){
        std::cerr << "Error: direccion fuera del rango \n";
    }else{
        // Escribe el dato en la dirección
        storage[address] = data;
    }
}

void Memory::initialize(int address, double data) {
    
    // Verificar que la dirección está dentro del rango
    int err = Memory::error(address);
    if (err == -1){
        std::cerr << "Error: direccion fuera del rango \n";
    }else{
        // Escribe el dato en la dirección
        storage[address] = data;
    }
}

// Imprimir el contenido de memoria y los accesos a memoria
void Memory::printMemory() const {
    std::cout << "\n MEMORY CONTENTS " << std::endl;
    for (size_t i = 0; i < storage.size(); ++i) {
        std::cout << "Address " << i << ": " << storage[i] << std::endl;
    }
    std::cout << "Total memory accesses: " << access_count << std::endl;
}

// Verificar que la dirección esté en el rango indicado
int Memory::error(int address){
    if (address < 0 || address >= MEM_SIZE){
        return -1;
    }else{
        return 0;
    }
}


// Main de Pueba

int main() {
    Memory mem;

    mem.initialize(10, 42);
    mem.write(20, 99);

    std::cout << "Valor en 10: " << mem.read(10) << std::endl;
    std::cout << "Valor en 20: " << mem.read(20) << std::endl;

    mem.printMemory();
    std::cout << "Accesos: " << mem.getAccessCount() << std::endl;

    return 0;
}
