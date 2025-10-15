#include "interconnect.h"
#include "processing_element.h"
#include "snoop.h"
#include <vector>

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

  pes[0]->mov(0, 0);
  pes[0]->mov(1, 8);
  pes[0]->mov(2, 16);
  pes[0]->mov(3, 24);

  pes[0]->printStatus();

  memory->printMemory();

  pes[0]->load(0, 0);
  pes[0]->load(1, 1);
  pes[0]->load(2, 2);
  pes[0]->load(3, 3);

  pes[0]->printStatus();

  memory->printMemory();

  pes[1]->mov(0, 32);
  pes[1]->mov(1, 40);
  pes[1]->mov(2, 48);
  pes[1]->mov(3, 56);

  pes[1]->printStatus();

  pes[1]->load(0, 0);
  pes[1]->load(1, 1);
  pes[1]->load(2, 2);
  pes[1]->load(3, 3);

  pes[1]->printStatus();

  pes[0]->mov(4, 2);
  pes[0]->fmul(0, 0, 4);
  pes[0]->fmul(1, 1, 4);
  pes[0]->fmul(2, 2, 4);
  pes[0]->fmul(3, 3, 4);

  pes[0]->printStatus();

  pes[0]->mov(4, 0);
  pes[0]->mov(5, 8);
  pes[0]->mov(6, 16);
  pes[0]->mov(7, 24);

  pes[0]->printStatus();

  pes[0]->store(0, 4);
  pes[0]->store(1, 5);
  pes[0]->store(2, 6);
  pes[0]->store(3, 7);

  pes[0]->printStatus();

  memory->printMemory();

  for (auto pe : pes) {
    delete pe;
  }
  delete bus;
  delete memory;

  return 0;
}
