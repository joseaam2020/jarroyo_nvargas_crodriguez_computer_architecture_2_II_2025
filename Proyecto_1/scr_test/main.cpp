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

    std::cout << "\n\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║                    SIMULATION START                      ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    std::cout << "\n--- Test 1: PE0 reads address 100 (should be EXCLUSIVE) ---";
    pes[0]->read(100);

    std::cout << "\n--- Test 2: PE1 reads address 100 (should be SHARED) ---";
    pes[1]->read(100);

    std::cout << "\n--- Test 3: PE2 reads address 100 (should be SHARED) ---";
    pes[2]->read(100);

    std::cout << "\n--- Test 4: PE0 writes to address 100 (SHARED->MODIFIED, invalidate others) ---";
    pes[0]->write(100, 999);

    std::cout << "\n--- Test 5: PE1 reads address 100 (PE0 flushes, both become SHARED) ---";
    pes[1]->read(100);

    std::cout << "\n--- Test 6: PE3 writes to address 200 (INVALID->MODIFIED) ---";
    pes[3]->write(200, 888);

    std::cout << "\n--- Test 7: PE0 reads address 300 (should be EXCLUSIVE) ---";
    pes[0]->read(300);

    std::cout << "\n--- Test 8: PE0 writes to address 300 (EXCLUSIVE->MODIFIED) ---";
    pes[0]->write(300, 777);

    std::cout << "\n--- Test 9: PE2 reads address 300 (PE0 flushes MODIFIED data) ---";
    pes[2]->read(300);

    std::cout << "\n--- Test 10: Multiple PEs read address 400 ---";
    pes[0]->read(400);
    pes[1]->read(400);
    pes[2]->read(400);
    pes[3]->read(400);

    printSystemState(pes, memory);

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
