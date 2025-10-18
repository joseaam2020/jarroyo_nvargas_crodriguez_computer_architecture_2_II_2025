#include "interconnect.h"
#include "processing_element.h"
#include "snoop.h"
#include <vector>
#include <thread>

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
        // Cada PE hace cosas distintas, se identifica con su ID
        int id = pe->getPEId(); 

        if (id == 0) {
            // PE0 ejecuta estas instrucciones (EJEMPLO)
            pe->mov(0, 0);
            pe->mov(1, 8);
            pe->load(0, 0);
            pe->load(1, 1);
            pe->fmul(2, 0, 1);
            pe->store(2, 16);
            std::cout << "[PE0] terminó su ejecución.\n";
        } 
        else if (id == 1) {
            // PE1 ejecuta otras instrucciones (EJEMPLO)
            pe->mov(0, 32);
            pe->mov(1, 40);
            pe->load(0, 0);
            pe->load(1, 1);
            pe->fadd(2, 0, 1);
            pe->store(2, 48);
            std::cout << "[PE1] terminó su ejecución.\n";
        }

        pe->printStatus();
    }); 
  }

  // Esperar a que todos los hilos terminen
  for (auto &t : threads) {
      t.join();
  }

  memory->printMemory(); // No sé si esto va a aquí o en otro lado

  for (auto pe : pes) {
    delete pe;
  }
  delete bus;
  delete memory;

  return 0;
}
