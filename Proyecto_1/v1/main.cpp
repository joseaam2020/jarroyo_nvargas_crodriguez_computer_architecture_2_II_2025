#include "interconnect.h"
#include "processing_element.h"
#include "snoop.h"
#include <vector>
#include <thread>
#include <mutex> 

std::mutex print_mutex;

int main() {

  std::cout << "\n";
  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║          MESI CACHE COHERENCE PROTOCOL SIMULATOR         ║\n";
  std::cout << "║                    2 Processing Elements                 ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";
  std::cout << std::endl;

  Memory *memory = new Memory();
  memory->initialize(0, 15.0);
  memory->initialize(8, 13.9);
  memory->initialize(16, 41.3);
  memory->initialize(24, 100.8);

  memory->initialize(32, 30.0);
  memory->initialize(40, 27.8);
  memory->initialize(48, 82.6);
  memory->initialize(56, 201.6);

  Interconnect *bus = new Interconnect(memory);

  std::vector<ProcessingElement *> pes;
  for (int i = 0; i < 2; i++) {
    ProcessingElement *pe = new ProcessingElement(i, bus);
    bus->registerSnoopModule(pe->getSnoop());
    pes.push_back(pe);
  }

  memory->printMemory();

  std::cout << "\n\n";
  std::cout << "╔══════════════════════════════════════════════════════════╗\n";
  std::cout << "║                    SIMULATION START                      ║\n";
  std::cout << "╚══════════════════════════════════════════════════════════╝\n";

  std::vector<std::thread> threads;

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
          pe->printStatus();
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
          pe->printStatus();
        }
      }
    });
  }

  // Esperar a que todos los hilos terminen
  for (auto &t : threads) {
    t.join();
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
}