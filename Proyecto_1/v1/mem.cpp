#include <iostream>
#include <cstdint>  // para uint64_t
#include <memory>   // para smart pointers, opcional

#define MEM_SIZE 512  // número de posiciones de memoria

int main() {
    // Crear memoria principal dinámica usando un puntero inteligente
    std::unique_ptr<uint64_t[]> memory(new uint64_t[MEM_SIZE]);

    // Inicializar toda la memoria a cero
    for (int i = 0; i < MEM_SIZE; i++) {
        memory[i] = 0;
    }

    // Se definen los segmentos con su posición inicial y posición final

    // Ejemplo de escritura
    memory[0] = 0x123456789ABCDEF0;
    memory[10] = 0xFEDCBA9876543210;

    // Ejemplo de lectura
    std::cout << "memory[0]  = 0x" << std::hex << memory[0] << std::endl;
    std::cout << "memory[10] = 0x" << std::hex << memory[10] << std::endl;

    // No es necesario liberar memoria manualmente cuando se usa std::unique_ptr

    return 0;
}
