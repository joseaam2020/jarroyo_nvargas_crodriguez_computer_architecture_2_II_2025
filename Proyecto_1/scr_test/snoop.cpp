#include "snoop.h"
#include <iostream>

SnoopModule::SnoopModule(int id, Cache* c) : pe_id(id), cache(c) {}

SnoopModule::SnoopResponse SnoopModule::handleBusRead(int address) {
    SnoopResponse response;

    if (!cache->hasLine(address)) {
        return response;
    }

    MESIState state = cache->getState(address);
    response.hit = true;
    response.data = cache->getData(address);

    switch(state) {
        case MESIState::MODIFIED:
            std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in MODIFIED -> Flush to memory & transition to SHARED" << std::endl;
            response.modified = true;
            response.shared = true;
            cache->setState(address, MESIState::SHARED);
            break;

        case MESIState::EXCLUSIVE:
            std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in EXCLUSIVE -> transition to SHARED" << std::endl;
            response.shared = true;
            cache->setState(address, MESIState::SHARED);
            break;

        case MESIState::SHARED:
            std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in SHARED -> remain SHARED" << std::endl;
            response.shared = true;
            break;

        case MESIState::INVALID:
            response.hit = false;
            break;
    }

    return response;
}

SnoopModule::SnoopResponse SnoopModule::handleBusReadX(int address) {
    SnoopResponse response;

    if (!cache->hasLine(address)) {
        return response;
    }

    MESIState state = cache->getState(address);
    response.hit = true;
    response.data = cache->getData(address);

    switch(state) {
        case MESIState::MODIFIED:
            std::cout << "  [PE" << pe_id << " Snoop] BusReadX hit in MODIFIED -> Flush to memory & INVALIDATE" << std::endl;
            response.modified = true;
            cache->invalidateLine(address);
            break;

        case MESIState::EXCLUSIVE:
            std::cout << "  [PE" << pe_id << " Snoop] BusReadX hit in EXCLUSIVE -> INVALIDATE" << std::endl;
            cache->invalidateLine(address);
            break;

        case MESIState::SHARED:
            std::cout << "  [PE" << pe_id << " Snoop] BusReadX hit in SHARED -> INVALIDATE" << std::endl;
            cache->invalidateLine(address);
            break;

        case MESIState::INVALID:
            response.hit = false;
            break;
    }

    return response;
}

void SnoopModule::handleBusInvalidate(int address) {
    if (cache->hasLine(address)) {
        std::cout << "  [PE" << pe_id << " Snoop] Invalidate signal -> INVALIDATE line" << std::endl;
        cache->invalidateLine(address);
    }
}

void SnoopModule::processLocalRead(int address, bool shared_signal, int data_from_source) {
    if (cache->hasLine(address)) {
        return;
    }

    MESIState new_state = shared_signal ? MESIState::SHARED : MESIState::EXCLUSIVE;
    cache->insertLine(address, data_from_source, new_state);

    std::cout << "  [PE" << pe_id << " Local] Read miss -> Load from source as "
              << stateToString(new_state) << std::endl;
}

void SnoopModule::processLocalWrite(int address, int data) {
    MESIState current_state = cache->getState(address);

    switch(current_state) {
        case MESIState::MODIFIED:
            cache->setData(address, data);
            std::cout << "  [PE" << pe_id << " Local] Write hit in MODIFIED -> Update data" << std::endl;
            break;

        case MESIState::EXCLUSIVE:
            cache->setData(address, data);
            cache->setState(address, MESIState::MODIFIED);
            std::cout << "  [PE" << pe_id << " Local] Write hit in EXCLUSIVE -> MODIFIED" << std::endl;
            break;

        case MESIState::SHARED:
            cache->setData(address, data);
            cache->setState(address, MESIState::MODIFIED);
            std::cout << "  [PE" << pe_id << " Local] Write hit in SHARED -> Invalidate others & MODIFIED" << std::endl;
            break;

        case MESIState::INVALID:
            cache->insertLine(address, data, MESIState::MODIFIED);
            std::cout << "  [PE" << pe_id << " Local] Write miss -> Load & MODIFIED" << std::endl;
            break;
    }
}

void SnoopModule::transitionOnRead(int address, bool other_caches_have) {
    if (other_caches_have) {
        cache->setState(address, MESIState::SHARED);
    } else {
        cache->setState(address, MESIState::EXCLUSIVE);
    }
}

void SnoopModule::transitionOnWrite(int address) {
    cache->setState(address, MESIState::MODIFIED);
}
