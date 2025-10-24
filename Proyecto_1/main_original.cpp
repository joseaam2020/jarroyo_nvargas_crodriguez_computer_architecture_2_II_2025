#include "interconnect.h"
#include "processing_element.h"
#include "snoop.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex print_mutex;

int main() {

  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║          MESI CACHE COHERENCE PROTOCOL SIMULATOR         ║\n";
  std::cout << "║           Vector Multiplication (2 PEs)                  ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";
  std::cout << std::endl;

  // Inicializar memoria
  Memory *memory = new Memory();

  // Vector A en direcciones 0-24 (4 elementos, stride 8)
  memory->initialize(0, 2.0);  // A[0]
  memory->initialize(8, 3.0);  // A[1]
  memory->initialize(16, 4.0); // A[2]
  memory->initialize(24, 5.0); // A[3]

  // Vector B en direcciones 32-56 (4 elementos, stride 8)
  memory->initialize(32, 1.5); // B[0]
  memory->initialize(40, 2.5); // B[1]
  memory->initialize(48, 3.5); // B[2]
  memory->initialize(56, 4.5); // B[3]

  // Vector C (resultado) en direcciones 64-88 (inicializado en 0)
  memory->initialize(64, 0.0); // C[0]
  memory->initialize(72, 0.0); // C[1]
  memory->initialize(80, 0.0); // C[2]
  memory->initialize(88, 0.0); // C[3]

  Interconnect *bus = new Interconnect(memory);

  std::vector<ProcessingElement *> pes;
  for (int i = 0; i < 2; i++) {
    ProcessingElement *pe = new ProcessingElement(i, bus);
    bus->registerSnoopModule(pe->getSnoop());
    pes.push_back(pe);
  }

  std::vector<std::thread> threads;

  memory->printMemory(); // No sé si esto va a aquí o en otro lado

  for (auto pe : pes) {
    threads.emplace_back([pe]() {
      int id = pe->getPEId();

      if (id == 0) {
        pe->mov(0, 0);
        pe->mov(1, 8);
        pe->load(0, 0);
        pe->load(1, 1);
        pe->fmul(2, 0, 1);
        pe->mov(3, 16);
        pe->store(2, 3);

        // 🔒 Protegemos la salida
        {
          std::lock_guard<std::mutex> lock(print_mutex);
          std::cout << "[PE0] terminó su ejecución.\n";
          //pe->printStatus();
        }
      } else if (id == 1) {
        pe->mov(0, 0);
        pe->mov(1, 40);
        pe->load(0, 0);
        pe->load(1, 1);
        pe->fadd(2, 0, 1);
        pe->mov(3, 24);
        pe->store(2, 3);

        // 🔒 Protegemos la salida
        {
          std::lock_guard<std::mutex> lock(print_mutex);
          std::cout << "[PE1] terminó su ejecución.\n";
          //pe->printStatus();
        }
      }
    });
  }

  

  // Esperar a que todos los hilos terminen
  for (auto &t : threads) {
    t.join();
  }

  for (auto pe : pes){
    pe->printStatus();
    
  }

  // 🔒 Protegemos el print final
  {
    std::lock_guard<std::mutex> lock(print_mutex);
    memory->printMemory();
  }

  memory->printMemory(); // No sé si esto va a aquí o en otro lado

  // Limpieza
  for (auto pe : pes) {
    delete pe;
  }
  delete bus;
  delete memory;

  return 0;


  std::cout << "\n📋 MEMORIA INICIAL:\n";
  memory->printMemory();

  std::cout << "\n\n";
  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║                    SIMULATION START                      ║\n";
  std::cout << "║  Operación: C[i] = A[i] * B[i] (multiplicación)         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";

  // ========================================================================
  // FASE 1: PE0 procesa elementos 0 y 1 (primeros dos elementos)
  // Estado EXCLUSIVE cuando PE0 lee por primera vez
  // ========================================================================
  std::cout
      << "\n\n┌─────────────────────────────────────────────────────────┐\n";
  std::cout << "│ FASE 1: PE0 procesa A[0]*B[0] y A[1]*B[1]              │\n";
  std::cout << "│ Esperado: Estados EXCLUSIVE → MODIFIED                 │\n";
  std::cout << "└─────────────────────────────────────────────────────────┘\n";

  // PE0 carga direcciones de A[0] y A[1]
  std::cout
      << "\n→ PE0: Configurando direcciones de vector A (elementos 0,1)\n";
  pes[0]->mov(4, 0); // R4 = dirección A[0]
  pes[0]->mov(5, 8); // R5 = dirección A[1]

  // PE0 carga A[0] y A[1] (INVALID → EXCLUSIVE)
  std::cout << "\n→ PE0: Cargando A[0] y A[1] (INVALID → EXCLUSIVE)\n";
  pes[0]->load(0, 4); // R0 = A[0] = 2.0
  pes[0]->load(1, 5); // R1 = A[1] = 3.0

  pes[0]->printStatus();

  // PE0 carga direcciones de B[0] y B[1]
  std::cout
      << "\n→ PE0: Configurando direcciones de vector B (elementos 0,1)\n";
  pes[0]->mov(6, 32); // R6 = dirección B[0]
  pes[0]->mov(7, 40); // R7 = dirección B[1]

  // PE0 carga B[0] y B[1] (INVALID → EXCLUSIVE)
  std::cout << "\n→ PE0: Cargando B[0] y B[1] (INVALID → EXCLUSIVE)\n";
  pes[0]->load(2, 6); // R2 = B[0] = 1.5
  pes[0]->load(3, 7); // R3 = B[1] = 2.5

  // PE0 multiplica
  std::cout << "\n→ PE0: Realizando multiplicaciones\n";
  pes[0]->fmul(0, 0, 2); // R0 = A[0] * B[0] = 3.0
  pes[0]->fmul(1, 1, 3); // R1 = A[1] * B[1] = 7.5

  pes[0]->printStatus();

  // PE0 guarda resultados en C[0] y C[1] (EXCLUSIVE → MODIFIED)
  std::cout << "\n→ PE0: Guardando resultados en C[0] y C[1] (EXCLUSIVE → "
               "MODIFIED)\n";
  pes[0]->mov(4, 64);  // R4 = dirección C[0]
  pes[0]->mov(5, 72);  // R5 = dirección C[1]
  pes[0]->store(0, 4); // C[0] = 3.0
  pes[0]->store(1, 5); // C[1] = 7.5

  pes[0]->printStatus();

  std::cout << "\n MEMORIA después de PE0 (fase 1):\n";
  memory->printMemory();

  // ========================================================================
  // FASE 2: PE1 procesa elementos 2 y 3
  // Estado EXCLUSIVE cuando PE1 lee por primera vez
  // ========================================================================
  std::cout
      << "\n\n┌─────────────────────────────────────────────────────────┐\n";
  std::cout << "│ FASE 2: PE1 procesa A[2]*B[2] y A[3]*B[3]              │\n";
  std::cout << "│ Esperado: Estados EXCLUSIVE → MODIFIED                 │\n";
  std::cout << "└─────────────────────────────────────────────────────────┘\n";

  // PE1 carga direcciones de A[2] y A[3]
  std::cout
      << "\n→ PE1: Configurando direcciones de vector A (elementos 2,3)\n";
  pes[1]->mov(4, 16); // R4 = dirección A[2]
  pes[1]->mov(5, 24); // R5 = dirección A[3]

  // PE1 carga A[2] y A[3] (INVALID → EXCLUSIVE)
  std::cout << "\n→ PE1: Cargando A[2] y A[3] (INVALID → EXCLUSIVE)\n";
  pes[1]->load(0, 4); // R0 = A[2] = 4.0
  pes[1]->load(1, 5); // R1 = A[3] = 5.0

  // PE1 carga direcciones de B[2] y B[3]
  std::cout
      << "\n→ PE1: Configurando direcciones de vector B (elementos 2,3)\n";
  pes[1]->mov(6, 48); // R6 = dirección B[2]
  pes[1]->mov(7, 56); // R7 = dirección B[3]

  // PE1 carga B[2] y B[3] (INVALID → EXCLUSIVE)
  std::cout << "\n→ PE1: Cargando B[2] y B[3] (INVALID → EXCLUSIVE)\n";
  pes[1]->load(2, 6); // R2 = B[2] = 3.5
  pes[1]->load(3, 7); // R3 = B[3] = 4.5

  // PE1 multiplica
  std::cout << "\n→ PE1: Realizando multiplicaciones\n";
  pes[1]->fmul(0, 0, 2); // R0 = A[2] * B[2] = 14.0
  pes[1]->fmul(1, 1, 3); // R1 = A[3] * B[3] = 22.5

  pes[1]->printStatus();

  // PE1 guarda resultados en C[2] y C[3] (EXCLUSIVE → MODIFIED)
  std::cout << "\n→ PE1: Guardando resultados en C[2] y C[3] (EXCLUSIVE → "
               "MODIFIED)\n";
  pes[1]->mov(4, 80);  // R4 = dirección C[2]
  pes[1]->mov(5, 88);  // R5 = dirección C[3]
  pes[1]->store(0, 4); // C[2] = 14.0
  pes[1]->store(1, 5); // C[3] = 22.5

  pes[1]->printStatus();

  std::cout << "\n📋 MEMORIA después de PE1 (fase 2):\n";
  memory->printMemory();

  // ========================================================================
  // FASE 3: PE0 lee datos que PE1 tiene en MODIFIED
  // Estado MODIFIED → SHARED (write-back) → PE0 obtiene SHARED
  // ========================================================================
  std::cout
      << "\n\n┌─────────────────────────────────────────────────────────┐\n";
  std::cout << "│ FASE 3: PE0 lee C[2] que PE1 tiene en MODIFIED         │\n";
  std::cout << "│ Esperado: PE1 MODIFIED → SHARED, PE0 INVALID → SHARED  │\n";
  std::cout << "└─────────────────────────────────────────────────────────┘\n";

  std::cout << "\n→ PE0: Leyendo C[2] (debe causar write-back de PE1)\n";
  pes[0]->mov(6, 80); // R6 = dirección C[2]
  pes[0]->load(2, 6); // R2 = C[2] (PE1 tiene esta línea en MODIFIED)

  pes[0]->printStatus();
  pes[1]->printStatus();

  // ========================================================================
  // FASE 4: PE1 lee datos que PE0 tiene en MODIFIED
  // Estado MODIFIED → SHARED
  // ========================================================================
  std::cout
      << "\n\n┌─────────────────────────────────────────────────────────┐\n";
  std::cout << "│ FASE 4: PE1 lee C[0] que PE0 tiene en MODIFIED         │\n";
  std::cout << "│ Esperado: PE0 MODIFIED → SHARED, PE1 INVALID → SHARED  │\n";
  std::cout << "└─────────────────────────────────────────────────────────┘\n";

  std::cout << "\n→ PE1: Leyendo C[0] (debe causar write-back de PE0)\n";
  pes[1]->mov(6, 64); // R6 = dirección C[0]
  pes[1]->load(2, 6); // R2 = C[0] (PE0 tiene esta línea en MODIFIED)

  pes[0]->printStatus();
  pes[1]->printStatus();

  // ========================================================================
  // FASE 5: PE0 escribe sobre línea SHARED
  // Estado SHARED → MODIFIED, PE1: SHARED → INVALID
  // ========================================================================
  std::cout
      << "\n\n┌─────────────────────────────────────────────────────────┐\n";
  std::cout << "│ FASE 5: PE0 escribe en C[0] que está en SHARED         │\n";
  std::cout << "│ Esperado: PE0 SHARED → MODIFIED, PE1 SHARED → INVALID  │\n";
  std::cout << "└─────────────────────────────────────────────────────────┘\n";

  std::cout << "\n→ PE0: Modificando C[0] (duplicar resultado)\n";
  pes[0]->mov(7, 2);     // R7 = 2
  pes[0]->load(0, 4);    // R0 = C[0] actual
  pes[0]->fmul(0, 0, 7); // R0 = C[0] * 2
  pes[0]->store(0, 4);   // C[0] = resultado duplicado (SHARED → MODIFIED)

  pes[0]->printStatus();
  pes[1]->printStatus();

  std::cout << "\n MEMORIA FINAL:\n";
  memory->printMemory();

  // ========================================================================
  // RESUMEN DE RESULTADOS
  // ========================================================================
  std::cout << "\n\n";
  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║                    RESULTADOS FINALES                    ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";
  std::cout << "\n✓ Multiplicación de vectores completada\n";
  std::cout << "✓ Todos los estados MESI ejercitados:\n";
  std::cout << "  • INVALID → EXCLUSIVE (lecturas iniciales)\n";
  std::cout << "  • EXCLUSIVE → MODIFIED (escrituras)\n";
  std::cout << "  • MODIFIED → SHARED (lecturas con write-back)\n";
  std::cout << "  • SHARED → MODIFIED (escrituras sobre compartido)\n";
  std::cout << "  • SHARED → INVALID (invalidaciones)\n";
}
