#include <iostream>
#include <vector>
#include "memory.h"
#include "interconnect.h"
#include "processing_element.h"

void printSystemState(const std::vector<ProcessingElement*>& pes, Memory* memory) {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "SYSTEM STATE" << std::endl;
    std::cout << std::string(60, '=') << std::endl;

    for (auto pe : pes) {
        pe->printStatus();
        std::cout << std::endl;
    }

    memory->printMemory();
    std::cout << std::string(60, '=') << std::endl;
}

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║          MESI CACHE COHERENCE PROTOCOL SIMULATOR         ║\n";
    std::cout << "║                    4 Processing Elements                 ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << std::endl;

    // ===================== Inicialización del sistema =====================
    Memory* memory = new Memory();
    memory->initialize(100, 50);
    memory->initialize(200, 75);
    memory->initialize(300, 100);
    memory->initialize(400, 125);

    Interconnect* bus = new Interconnect(memory);

    std::vector<ProcessingElement*> pes;
    for (int i = 0; i < 4; i++) {
        ProcessingElement* pe = new ProcessingElement(i, bus);
        bus->registerSnoopModule(pe->getSnoop());
        pes.push_back(pe);
    }

    std::cout << "\nInitial memory state:" << std::endl;
    std::cout << "  Address 100: 50" << std::endl;
    std::cout << "  Address 200: 75" << std::endl;
    std::cout << "  Address 300: 100" << std::endl;
    std::cout << "  Address 400: 125" << std::endl;

    // ===================== Inicio de la simulación =====================
    std::cout << "\n\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║            SIMULATION: VECTOR MULTIPLICATION             ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    // ===================================================================
    // Simulación del código ensamblador:
    // load r4, [r2]
    // loop:
    //   load r5, [r0]
    //   load r6, [r1]
    //   fmul r7, r5, r6
    //   fadd r4, r4, r7
    //   inc r0, inc r1, dec r3
    //   jnz loop
    // stor r4, [r2]
    // halt
    // ===================================================================

    ProcessingElement* pe_exec = pes[0]; // ejecuta el programa PE0

    int A_base = 100;
    int B_base = 200;
    int partial_sum_addr = 300;
    int count = 4;  // elementos a procesar
    int r4 = 0;     // acumulador local
    int r5, r6, r7;

    std::cout << "\n[Instrucción] load r4, [r2]   ; r4 = partial_sum inicial (acumulador local)";
    pe_exec->read(partial_sum_addr);
    r4 = memory->read(partial_sum_addr);

    while (count > 0) {
        std::cout << "\n[Instrucción] load r5, [r0]   ; r5 = A[i]";
        pe_exec->read(A_base);
        r5 = memory->read(A_base);

        std::cout << "\n[Instrucción] load r6, [r1]   ; r6 = B[i]";
        pe_exec->read(B_base);
        r6 = memory->read(B_base);

        std::cout << "\n[Instrucción] fmul r7, r5, r6 ; r7 = A[i] * B[i]";
        r7 = r5 * r6;

        std::cout << "\n[Instrucción] fadd r4, r4, r7 ; r4 += r7";
        r4 += r7;

        std::cout << "\n[Instrucción] inc r0, inc r1, dec r3 ; avanzar posiciones";
        A_base += 1;
        B_base += 1;
        count -= 1;
    }

    std::cout << "\n[Instrucción] stor r4, [r2]   ; guarda partial_sum actualizado";
    pe_exec->write(partial_sum_addr, r4);

    std::cout << "\n[Instrucción] halt";
    std::cout << "\nPrograma finalizado.\n";

    // Mostrar estado del sistema MESI tras ejecutar las instrucciones
    printSystemState(pes, memory);

    // ===================== Fin de la simulación =====================
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    SIMULATION END                        ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";
    std::cout << std::endl;

    for (auto pe : pes) {
        delete pe;
    }
    delete bus;
    delete memory;

    return 0;
}
