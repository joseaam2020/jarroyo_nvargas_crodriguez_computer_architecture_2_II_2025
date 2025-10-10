#include "snoop.h"
#include "cache.h"
#include "interconnect.h"
#include <iostream>
#include <iomanip>

// Implementación del clase
SnoopModule::SnoopModule(int id, Cache* c, Interconnect* ic) 
    : pe_id(id), cache(c), interconnect(ic) {
    std::cout << "[SNOOP " << pe_id << "] Inicializado" << std::endl;
}

// Calcula los espacios de la caché
void SnoopModule::calculateAddressFields(int addr, int& word_index, int& offset,
                                         int& block_number, int& index, int& tag) {
    word_index = addr / 8;
    offset = word_index % 4; // posición en el bloque (0 a 3)
    block_number = word_index / 4;
    index = block_number % 8; // set index (0 a 7)
    tag = addr >> 8; // parte alta de la dirección
}

// Peticiones externas
// Otro PE quiere leer
SnoopModule::SnoopResponse SnoopModule::handleBusRead(int address) {
    SnoopResponse response;
    
    // Verificar si tenemos la línea en caché
    if (!cache->hasLine(address)) {
        return response; // hit = false
    }
    
    // Obtener estado actual
    mesi_state state = cache->getState(address);
    response.hit = true;

    int word_index = address / 8;
    int offset = word_index % 4; // posición en el bloque (0 a 3)
    int block_number = word_index / 4;
    int index = block_number % 8; // set index (0 a 7)
    int tag = address >> 8; // parte alta de la dirección
    
    // Calcular dirección del inicio del bloque
    int block_start_addr = (address / 32) * 32; // 32 bytes por bloque
    
    // Llenar el array de datos con todo el bloque
    for(int i = 0; i < 4; i++) {
        try {
            response.data[i] = cache->getData(block_start_addr + i * 8);
        } catch(...) {
            response.data[i] = 0.0;
        }
    }
    
    // Manejar transiciones según el estado actual
    switch(state) {
        case mesi_state::MODIFIED:
            std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in MODIFIED -> "
                      << "Flush to memory & transition to SHARED" << std::endl;
            response.modified = true;
            response.shared = true;
            cache->setState(address, mesi_state::SHARED);
            break;
            
        case mesi_state::EXCLUSIVE:
            std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in EXCLUSIVE -> "
                      << "transition to SHARED" << std::endl;
            response.shared = true;
            cache->setState(address, mesi_state::SHARED);
            break;
            
        case mesi_state::SHARED:
            std::cout << "  [PE" << pe_id << " Snoop] BusRead hit in SHARED -> "
                      << "remain SHARED" << std::endl;
            response.shared = true;
            break;
            
        case mesi_state::INVALID:
            response.hit = false;
            break;
    }
    
    return response;
}


// Invalidar porque otro PE está escribiendo
void SnoopModule::handleBusInvalidate(int address) {
    if (cache->hasLine(address)) {
        std::cout << "  [PE" << pe_id << " Snoop] Invalidate signal -> "
                  << "INVALIDATE line" << std::endl;
        cache->invalidateLine(address);
    }
}

// Cambios internos
// Lectura local después de un miss
void SnoopModule::processLocalRead(int address, bool shared_signal, double* data_from_source) {
    // Si ya tenemos la línea, no hacer nada (read hit)
    if (cache->hasLine(address)) {
        return;
    }
    
    // Determinar nuevo estado
    mesi_state new_state = shared_signal ? mesi_state::SHARED : mesi_state::EXCLUSIVE;
    
    // Insertar el bloque completo en la caché (cambiar porque ahorita es solo una línea)
    cache->insertLine(address, data_from_source[0], new_state);
    
}


// Escritura local (hit o miss)
void SnoopModule::processLocalWrite(int address, double value) {
    mesi_state current_state = cache->getState(address);
    
    switch(current_state) {
        case mesi_state::MODIFIED:
            // Si ya está en MODIFIED, solo actualizar el dato
            cache->setData(address, value);
            std::cout << "  [PE" << pe_id << " Local] Write hit in MODIFIED -> "
                      << "Update data" << std::endl;
            break;
            
        case mesi_state::EXCLUSIVE:
            // El dato está en Exclusivo, escribir y pasar a MODIFIED
            cache->setData(address, value);
            cache->setState(address, mesi_state::MODIFIED);
            std::cout << "  [PE" << pe_id << " Local] Write hit in EXCLUSIVE -> "
                      << "MODIFIED" << std::endl;
            break;
            
        case mesi_state::SHARED:
            // El dato está en otros PE, invalidar las otras copias y pasar a MODIFIED
            cache->setData(address, value);
            cache->setState(address, mesi_state::MODIFIED);
            std::cout << "  [PE" << pe_id << " Local] Write hit in SHARED -> "
                      << "Invalidate others & MODIFIED" << std::endl;
            break;
            
        case mesi_state::INVALID:
            // No tenemos el dato, se debe traer primero (write miss)
            cache->insertLine(address, value, mesi_state::MODIFIED);
            std::cout << "  [PE" << pe_id << " Local] Write miss -> "
                      << "Load & MODIFIED" << std::endl;
            break;
    }
}

// Parte de Interconnect

/**
 * Read Miss usando el interconnect
 * 
 * 1. Solicitar dato al interconnect (broadcastRead)
 * 2. Interconnect pregunta a otros snoops
 * 3. Recibir dato y determinar estado (SHARED o EXCLUSIVE)
 * 4. Insertar en caché local
 */
bool SnoopModule::handleReadMiss(int address) {
    if (!interconnect) {
        std::cerr << "[SNOOP " << pe_id << "] ERROR: No hay interconnect configurado" 
                  << std::endl;
        return false;
    }
    
    std::cout << "\n[SNOOP " << pe_id << "] Manejando Read miss para addr 0x" 
              << std::hex << address << std::dec << std::endl;
    
    // Paso 1: Solicitar datos al interconnect
    auto bus_result = interconnect->broadcastRead(pe_id, address);
    
    // Paso 2: Determinar estado basado en si está compartido
    bool shared_signal = bus_result.shared || bus_result.modified;
    mesi_state new_state = shared_signal ? mesi_state::SHARED : mesi_state::EXCLUSIVE;
    
    // Paso 3: Insertar línea en caché con el primer dato del bloque
    cache->insertLine(address, bus_result.data[0], new_state);
    
    
    return true;
}

/**
 * Maneja un write (hit o miss) usando el interconnect
 * 
 * 1. Si es MISS, traer dato primero
 * 2. Invalidar otras copias (broadcastInvalidate)
 * 3. Escribir dato en caché con estado MODIFIED
 */
bool SnoopModule::handleWrite(int address, double value) {
    if (!interconnect) {
        std::cerr << "[SNOOP " << pe_id << "] ERROR: No hay interconnect configurado" 
                  << std::endl;
        return false;
    }
    
    std::cout << "\n[SNOOP " << pe_id << "] Manejando WRITE para addr 0x" 
              << std::hex << address << std::dec << std::endl;
    
    // Estado actual de la línea de caché
              mesi_state current_state = cache->getState(address);
    
    // Caso 1: WRITE MISS (estado INVALID)
    if (current_state == mesi_state::INVALID) {
        std::cout << "  [SNOOP " << pe_id << "] WRITE MISS detectado" << std::endl;
        
        // Traer dato con intención de modificar
        auto bus_result = interconnect->broadcastRead(pe_id, address);
        
        // Insertar con estado MODIFIED
        cache->insertLine(address, value, mesi_state::MODIFIED);

        // Invalidar otras copias, ya que el procesador va a escribir
        interconnect->broadcastInvalidate(pe_id, address);
        
        std::cout << "  [SNOOP " << pe_id << "] WRITE MISS completado: estado MODIFIED" 
                  << std::endl;

    }
    // Caso 2: WRITE HIT (cualquier otro estado)
    else {
        
        // Si estamos en SHARED, se invalidan las otras copias
        if (current_state == mesi_state::SHARED) {
            interconnect->broadcastInvalidate(pe_id, address);
        }
        
        // Actualizar dato y cambiar a MODIFIED
        cache->setData(address, value);
        cache->setState(address, mesi_state::MODIFIED);
        
        std::cout << "  [SNOOP " << pe_id << "] WRITE HIT completado: estado MODIFIED" 
                  << std::endl;
    }
    
    return true;
}

// Transición de estado después de una lectura
void SnoopModule::transitionOnRead(int address, bool other_caches_have) {
    if (other_caches_have) {
        cache->setState(address, mesi_state::SHARED);
    } else {
        cache->setState(address, mesi_state::EXCLUSIVE);
    }
}

//Transición de estado después de una escritura
void SnoopModule::transitionOnWrite(int address) {
    cache->setState(address, mesi_state::MODIFIED);
}