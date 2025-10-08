#include "processing_element.h"
#include <iostream>

// Constructor del Processing Element (PE)
// Cada PE representa un procesador con su propia caché y un módulo snoop
// que observa las acciones de los demás PE en el bus compartido.
ProcessingElement::ProcessingElement(int id, Interconnect* interconnect)
    : pe_id(id), bus(interconnect) {
    cache = new Cache(id);               // Se crea la caché local del PE
    snoop = new SnoopModule(id, cache);  // Se asocia un módulo Snoop para mantener coherencia MESI
}

// Destructor: libera la memoria de la caché y del snoop.
ProcessingElement::~ProcessingElement() {
    delete snoop;
    delete cache;
}

// ------------------------------------------------------------
// Operación de lectura local en una dirección de memoria
// ------------------------------------------------------------
void ProcessingElement::read(int address) {
    std::cout << "\n>>> PE" << pe_id << " requests READ on address " << address << std::endl;

    // Si la línea ya está en caché, se tiene un HIT
    if (cache->hasLine(address)) {
        std::cout << "  [PE" << pe_id << "] Cache HIT - State: "
                  << stateToString(cache->getState(address))
                  << " Data: " << cache->getData(address) << std::endl;
        // En este caso no hay cambio de estado ni comunicación por el bus
        // (E, S o M se mantienen igual)
        return;
    }

    // Si no está en caché, se produce un MISS y se debe solicitar por el bus
    std::cout << "  [PE" << pe_id << "] Cache MISS - Broadcasting on bus" << std::endl;

    // Se envía una solicitud de lectura (BusRead) al bus compartido
    // Otros PE responderán si tienen la línea (S o M)
    auto result = bus->broadcastRead(pe_id, address);

    // El módulo snoop local decide si el nuevo estado será SHARED o EXCLUSIVE
    // dependiendo de si otros PE también la tenían
    snoop->processLocalRead(address, result.shared, result.data);

    // Se imprime el resultado final de la lectura
    std::cout << "  [PE" << pe_id << "] Read complete - Data: " << result.data
              << " State: " << stateToString(cache->getState(address)) << std::endl;
}

// ------------------------------------------------------------
// Operación de escritura local en una dirección de memoria
// ------------------------------------------------------------
void ProcessingElement::write(int address, int data) {
    std::cout << "\n>>> PE" << pe_id << " requests WRITE on address " << address << " with data " << data << std::endl;

    // Se obtiene el estado actual de la línea en la caché
    MESIState current_state = cache->getState(address);

    // --------------------------------------------------------
    // Caso 1: Estado INVALID
    // No se tiene la línea, se solicita con permiso de escritura
    // --------------------------------------------------------
    if (current_state == MESIState::INVALID) {
        std::cout << "  [PE" << pe_id << "] Cache MISS - Broadcasting READ_X on bus" << std::endl;
        // READ_X (Read Exclusive) solicita la línea y fuerza invalidaciones
        // en las demás cachés que la tengan
        auto result = bus->broadcastReadX(pe_id, address);
    }

    // --------------------------------------------------------
    // Caso 2: Estado SHARED
    // Se tiene una copia compartida, pero no se puede escribir todavía.
    // Debe invalidar las copias de los demás PE antes de escribir.
    // --------------------------------------------------------
    else if (current_state == MESIState::SHARED) {
        std::cout << "  [PE" << pe_id << "] Cache in SHARED - Broadcasting INVALIDATE" << std::endl;
        // Envia una señal de INVALIDATE en el bus para que las demás cachés
        // pasen a estado INVALID
        bus->broadcastInvalidate(pe_id, address);
    }

    // --------------------------------------------------------
    // Procesamiento local de la escritura
    // Actualiza el dato y cambia de estado según las reglas MESI:
    // E -> M, S -> M, I -> M, M -> M
    // --------------------------------------------------------
    snoop->processLocalWrite(address, data);

    // Se imprime el estado final después de la escritura
    std::cout << "  [PE" << pe_id << "] Write complete - Data: " << data
              << " State: " << stateToString(cache->getState(address)) << std::endl;
}

// ------------------------------------------------------------
// Muestra el estado completo de la caché del PE
// (direcciones, datos y estados MESI actuales)
// ------------------------------------------------------------
void ProcessingElement::printStatus() const {
    cache->printCache();
}
