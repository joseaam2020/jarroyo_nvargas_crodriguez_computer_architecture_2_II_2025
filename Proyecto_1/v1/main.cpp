#include "interconnect.h"
#include "processing_element.h"
#include "snoop.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// Vector de instrucciones (obtenido de instrucciones.txt)
std::vector<std::string> file_lines = {".PE0",
                                       "mov  r0, #16",
                                       "mov  r1, #2",
                                       "mov  r2, #3",
                                       "fmul r3, r1, r2",
                                       "break",
                                       "store r3, [r0]",
                                       "mov  r4, #6",
                                       "load r5, [r0]",
                                       "fadd r6, r5, r4",
                                       ".PE1",
                                       "mov  r0, #16",
                                       "mov  r1, #2",
                                       "mov  r2, #3",
                                       "fmul r3, r1, r2",
                                       "break",
                                       "store r3, [r0]",
                                       "mov  r4, #6",
                                       "load r5, [r0]",
                                       "fadd r6, r5, r4"};

std::mutex print_mutex;

int main() {
  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║          MESI CACHE COHERENCE PROTOCOL SIMULATOR         ║\n";
  std::cout << "║           Vector Multiplication (2 PEs)                  ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";
  std::cout << std::endl;

  int pe_amount = 2; // Cantidad de PEs a ejecutar
  // Dividir las intrucciones en varios vectores (para cada PE)
  // pe_instructions[#] va a tener las instrucciones de PE# (solo instrucciones)
  std::vector<std::vector<std::vector<std::string>>> pe_instructions(pe_amount);
  int current_pe = -1;

  for (const auto &line : file_lines) {
    if (line.rfind(".PE", 0) == 0) { // Si empieza con ".PE"
      current_pe =
          std::stoi(line.substr(3)); // Obtener el número de PE correspondiente
      pe_instructions[current_pe].push_back({}); // Crear el primer bloque
    } else if (current_pe != -1) {
      if (line == "break") {
        // Aquí, si no hay instrucciones después del break, se ignora
        pe_instructions[current_pe].push_back({});
      } else {
        // En caso de que hayan instrucciones, se agregar al final
        pe_instructions[current_pe].back().push_back(line);
      }
    }
  }

  // Print para verificar
  for (int i = 0; i < pe_amount; i++) {
    std::cout << "\n=== PE" << i << " ===\n";
    for (size_t b = 0; b < pe_instructions[i].size(); ++b) {
      if (pe_instructions[i][b].empty())
        continue;
      std::cout << "  Bloque " << b << ":\n";
      for (auto &instr : pe_instructions[i][b]) {
        std::cout << "    " << instr << "\n";
      }
    }
  }

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
  for (int i = 0; i < pe_amount; i++) {
    ProcessingElement *pe = new ProcessingElement(i, bus);
    bus->registerSnoopModule(pe->getSnoop());
    pes.push_back(pe);
  }

  std::cout << "\n MEMORIA INICIAL:\n";
  memory->printMemory();

  // En caso de regresar, el comentario grande de abajo va a aquí

  // Este código ejecuta la los hilos de cada PE en paralelo
  // Las intrucciones se ejecutan de forma secuencial
  std::vector<std::thread> threads;
  for (int i = 0; i < pe_amount; i++) {
    threads.emplace_back([&, i]() {
      for (const auto &block : pe_instructions[i]) {
        std::cout << "[Bloque]\n";
        for (const auto &instr : block) {
          pes[i]->execute(instr);

          std::lock_guard<std::mutex> lock(print_mutex);
        }
      }

      std::lock_guard<std::mutex> lock(print_mutex);
      // std::cout << "[PE" << i << "] ejecución finalizada.\n";
    });
  }

  // Esperar a que todos terminen
  for (auto &t : threads)
    t.join();

  for (auto pe : pes) {
    pe->printStatus();
  }

  std::cout << "\n MEMORIA FINAL:\n";
  memory->printMemory();

  // ─────────────── LIMPIEZA ───────────────
  for (auto pe : pes)
    delete pe;
  delete bus;
  delete memory;

  return 0;
}

/*    // ─────────────── SIMULATION START ───────────────
    std::cout << "\n\n";
    std::cout <<
"╔══════════════════════════════════════════════════════════╗\n"; std::cout <<
"║                    SIMULATION START                      ║\n"; std::cout <<
"║  Operación: C[i] = A[i] * B[i] (multiplicación)         ║\n"; std::cout <<
"╚══════════════════════════════════════════════════════════╝\n";
=======
  // ─────────────── SIMULATION START ───────────────
  std::cout << "\n\n";
  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║                    SIMULATION START                      ║\n";
  std::cout << "║  Operación: C[i] = A[i] * B[i] (multiplicación)         ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";
>>>>>>> Jose-Interfaz

  // ─────────────── FASE 1 ───────────────
  std::cout
      << "\n\n┌─────────────────────────────────────────────────────────┐\n";
  std::cout << "│ FASE 1: PE0 procesa A[0]*B[0] y A[1]*B[1]              │\n";
  std::cout << "│ Esperado: Estados EXCLUSIVE → MODIFIED                 │\n";
  std::cout << "└─────────────────────────────────────────────────────────┘\n";

  {
    std::vector<std::thread> threads;
    threads.emplace_back([&]() {
      // PE0
      pes[0]->mov(4, 0);
      pes[0]->mov(5, 8);
      pes[0]->load(0, 4);
      pes[0]->load(1, 5);
      pes[0]->mov(6, 32);
      pes[0]->mov(7, 40);
      pes[0]->load(2, 6);
      pes[0]->load(3, 7);
      pes[0]->fmul(0, 0, 2);
      pes[0]->fmul(1, 1, 3);
      pes[0]->mov(4, 64);
      pes[0]->mov(5, 72);
      pes[0]->store(0, 4);
      pes[0]->store(1, 5);

      std::lock_guard<std::mutex> lock(print_mutex);
      std::cout << "[PE0] Fase 1 completada\n";
    });

    for (auto &t : threads)
      t.join();

    for (auto pe : pes) {
      std::lock_guard<std::mutex> lock(print_mutex);
      pe->printStatus();
    }
  }

  // ─────────────── FASE 2 ───────────────
  std::cout
      << "\n\n┌─────────────────────────────────────────────────────────┐\n";
  std::cout << "│ FASE 2: PE1 procesa A[2]*B[2] y A[3]*B[3]              │\n";
  std::cout << "│ Esperado: Estados EXCLUSIVE → MODIFIED                 │\n";
  std::cout << "└─────────────────────────────────────────────────────────┘\n";

  {
    std::vector<std::thread> threads;
    threads.emplace_back([&]() {
      // PE1
      pes[1]->mov(4, 16);
      pes[1]->mov(5, 24);
      pes[1]->load(0, 4);
      pes[1]->load(1, 5);
      pes[1]->mov(6, 48);
      pes[1]->mov(7, 56);
      pes[1]->load(2, 6);
      pes[1]->load(3, 7);
      pes[1]->fmul(0, 0, 2);
      pes[1]->fmul(1, 1, 3);
      pes[1]->mov(4, 80);
      pes[1]->mov(5, 88);
      pes[1]->store(0, 4);
      pes[1]->store(1, 5);

      std::lock_guard<std::mutex> lock(print_mutex);
      std::cout << "[PE1] Fase 2 completada\n";
    });

    for (auto &t : threads)
      t.join();

    for (auto pe : pes) {
      std::lock_guard<std::mutex> lock(print_mutex);
      pe->printStatus();
    }
  }

  // ─────────────── FASE 3 ───────────────
  std::cout
      << "\n\n┌─────────────────────────────────────────────────────────┐\n";
  std::cout << "│ FASE 3: PE0 lee C[2] que PE1 tiene en MODIFIED         │\n";
  std::cout << "│ Esperado: PE1 MODIFIED → SHARED, PE0 INVALID → SHARED  │\n";
  std::cout << "└─────────────────────────────────────────────────────────┘\n";

  {
    std::vector<std::thread> threads;
    threads.emplace_back([&]() {
      pes[0]->mov(6, 80);
      pes[0]->load(2, 6);

      std::lock_guard<std::mutex> lock(print_mutex);
      std::cout << "[PE0] Fase 3 completada\n";
    });

 for (auto &t : threads) t.join();

        for (auto pe : pes) {
            std::lock_guard<std::mutex> lock(print_mutex);
            pe->printStatus();
        }

        {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "\n MEMORIA FINAL:\n";
            memory->printMemory();
        }
    }
*/
