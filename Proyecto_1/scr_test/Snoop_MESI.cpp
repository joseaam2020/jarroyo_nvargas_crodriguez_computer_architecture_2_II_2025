#include "snoop.h"
#include <iostream>

// ======================================================
// Clase: SnoopModule
// ------------------------------------------------------
// Este módulo representa el observador del bus (snooper)
// asociado a un procesador o PE. Su función es monitorear
// las transacciones de bus (lecturas/escrituras) generadas
// por otros procesadores y mantener la coherencia de la
// caché local aplicando las reglas del protocolo MESI.
// ======================================================

SnoopModule::SnoopModule(int id, Cache* c) : pe_id(id), cache(c) {}

// ------------------------------------------------------
// Maneja una transacción de lectura en el bus (BusRead).
// Esto ocurre cuando otro procesador intenta leer un bloque
// de memoria. El snoop debe verificar si su propia caché
// contiene ese bloque y reaccionar según el estado MESI.
// ------------------------------------------------------
SnoopModule::SnoopResponse SnoopModule::handleBusRead(int address) {
    SnoopResponse response;

    // Si la caché no tiene la línea, no hay nada que hacer.
    if (!cache->hasLine(address)) {
        return response;
    }

    MESIState state = cache->getState(address);
    response.hit = true;
    response.data = cache->getData(address);

    switch(state) {
        case MESIState::MODIFIED:
            // Si el bloque está modificado, se debe escribir (flush)
            // a memoria y cambiar el estado a SHARED, ya que otro PE
            // también lo va a leer.
            std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in MODIFIED -> Flush to memory & transition to SHARED" << std::endl;
            response.modified = true;
            response.shared = true;
            cache->setState(address, MESIState::SHARED);
            break;

        case MESIState::EXCLUSIVE:
            // Si estaba en EXCLUSIVE, otro PE lo está leyendo, por tanto
            // deja de ser exclusivo y pasa a SHARED.
            std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in EXCLUSIVE -> transition to SHARED" << std::endl;
            response.shared = true;
            cache->setState(address, MESIState::SHARED);
            break;

        case MESIState::SHARED:
            // Si ya estaba compartido, se mantiene en ese estado.
            std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in SHARED -> remain SHARED" << std::endl;
            response.shared = true;
            break;

        case MESIState::INVALID:
            // No tiene una copia válida, ignora el evento.
            response.hit = false;
            break;
    }

    return response;
}

// ------------------------------------------------------
// Maneja una transacción de lectura exclusiva (BusReadX),
// que normalmente ocurre antes de una escritura por parte
// de otro procesador. Según MESI, cualquier otra caché que
// tenga el bloque debe invalidar su copia.
// ------------------------------------------------------
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
            // Si estaba modificado, escribe el bloque a memoria
            // (flush) y luego invalida la línea local.
            std::cout << "  [PE" << pe_id << " Snoop] BusReadX hit in MODIFIED -> Flush to memory & INVALIDATE" << std::endl;
            response.modified = true;
            cache->invalidateLine(address);
            break;

        case MESIState::EXCLUSIVE:
            // Si estaba en EXCLUSIVE, invalida directamente.
            std::cout << "  [PE" << pe_id << " Snoop] BusReadX hit in EXCLUSIVE -> INVALIDATE" << std::endl;
            cache->invalidateLine(address);
            break;

        case MESIState::SHARED:
            // Si estaba compartido, también invalida.
            std::cout << "  [PE" << pe_id << " Snoop] BusReadX hit in SHARED -> INVALIDATE" << std::endl;
            cache->invalidateLine(address);
            break;

        case MESIState::INVALID:
            // Si ya estaba inválido, no hay acción.
            response.hit = false;
            break;
    }

    return response;
}

// ------------------------------------------------------
// Maneja una señal explícita de invalidación enviada por
// otro PE. Si la línea existe en la caché, se marca como
// inválida.
// ------------------------------------------------------
void SnoopModule::handleBusInvalidate(int address) {
    if (cache->hasLine(address)) {
        std::cout << "  [PE" << pe_id << " Snoop] Invalidate signal -> INVALIDATE line" << std::endl;
        cache->invalidateLine(address);
    }
}

// ------------------------------------------------------
// Maneja una lectura local (originada por este PE) que no
// se encontraba en caché (read miss). Si otro PE tiene el
// bloque, se carga en estado SHARED, de lo contrario en
// estado EXCLUSIVE.
// ------------------------------------------------------
void SnoopModule::processLocalRead(int address, bool shared_signal, int data_from_source) {
    if (cache->hasLine(address)) {
        return;
    }

    MESIState new_state = shared_signal ? MESIState::SHARED : MESIState::EXCLUSIVE;
    cache->insertLine(address, data_from_source, new_state);

    std::cout << "  [PE" << pe_id << " Local] Read miss -> Load from source as "
              << stateToString(new_state) << std::endl;
}

// ------------------------------------------------------
// Maneja una escritura local (write hit/miss). Cambia el
// estado del bloque según las reglas de MESI:
// - M: se actualiza el dato.
// - E: pasa a M.
// - S: invalida a otros y pasa a M.
// - I: se carga desde memoria y pasa a M.
// ------------------------------------------------------
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
            // Miss de escritura: se carga la línea y se marca como MODIFIED.
            cache->insertLine(address, data, MESIState::MODIFIED);
            std::cout << "  [PE" << pe_id << " Local] Write miss -> Load & MODIFIED" << std::endl;
            break;
    }
}

// ------------------------------------------------------
// Funciones auxiliares para ajustar el estado tras lecturas
// o escrituras locales, dependiendo de si otros cachés
// poseen el mismo bloque.
// ------------------------------------------------------
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
