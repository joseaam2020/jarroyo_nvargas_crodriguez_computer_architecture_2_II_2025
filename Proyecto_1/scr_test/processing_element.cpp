#include "processing_element.h"
#include <iostream>

ProcessingElement::ProcessingElement(int id, Interconnect* interconnect)
    : pe_id(id), bus(interconnect) {
    cache = new Cache(id);
    snoop = new SnoopModule(id, cache);
}

ProcessingElement::~ProcessingElement() {
    delete snoop;
    delete cache;
}

void ProcessingElement::read(int address) {
    std::cout << "\n>>> PE" << pe_id << " requests READ on address " << address << std::endl;

    if (cache->hasLine(address)) {
        std::cout << "  [PE" << pe_id << "] Cache HIT - State: "
                  << stateToString(cache->getState(address))
                  << " Data: " << cache->getData(address) << std::endl;
        return;
    }

    std::cout << "  [PE" << pe_id << "] Cache MISS - Broadcasting on bus" << std::endl;

    auto result = bus->broadcastRead(pe_id, address);

    snoop->processLocalRead(address, result.shared, result.data);

    std::cout << "  [PE" << pe_id << "] Read complete - Data: " << result.data
              << " State: " << stateToString(cache->getState(address)) << std::endl;
}

void ProcessingElement::write(int address, int data) {
    std::cout << "\n>>> PE" << pe_id << " requests WRITE on address " << address << " with data " << data << std::endl;

    MESIState current_state = cache->getState(address);

    if (current_state == MESIState::INVALID) {
        std::cout << "  [PE" << pe_id << "] Cache MISS - Broadcasting READ_X on bus" << std::endl;
        auto result = bus->broadcastReadX(pe_id, address);
    } else if (current_state == MESIState::SHARED) {
        std::cout << "  [PE" << pe_id << "] Cache in SHARED - Broadcasting INVALIDATE" << std::endl;
        bus->broadcastInvalidate(pe_id, address);
    }

    snoop->processLocalWrite(address, data);

    std::cout << "  [PE" << pe_id << "] Write complete - Data: " << data
              << " State: " << stateToString(cache->getState(address)) << std::endl;
}

void ProcessingElement::printStatus() const {
    cache->printCache();
}
