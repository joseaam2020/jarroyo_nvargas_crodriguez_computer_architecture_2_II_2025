#ifndef SNOOP_H
#define SNOOP_H

#include "mesi_protocol.h"  // Define los estados del protocolo MESI (Modified, Exclusive, Shared, Invalid)
#include "cache.h"          // Clase Cache, que contiene los bloques de memoria y sus estados

// Clase que representa el módulo Snoop (observador) del protocolo MESI
// Su función es "escuchar" el bus y reaccionar cuando otros procesadores (PE)
// realizan operaciones que afectan la coherencia de caché.
class SnoopModule {
private:
    Cache* cache;  // Puntero a la caché local asociada a este PE
    int pe_id;     // Identificador del elemento de procesamiento (Processing Element)

public:
    // Constructor: recibe el ID del PE y un puntero a su caché asociada
    SnoopModule(int id, Cache* c);

    // Estructura que define la respuesta del Snoop cuando detecta una operación en el bus
    struct SnoopResponse {
        bool hit;       // Indica si el bloque solicitado está presente en esta caché
        bool shared;    // Indica si el bloque está en estado compartido (S)
        bool modified;  // Indica si el bloque está en estado modificado (M)
        int data;       // Valor del bloque (si debe compartirse con otro PE)

        // Constructor por defecto: inicializa todos los valores a "falso" y el dato a 0
        SnoopResponse() : hit(false), shared(false), modified(false), data(0) {}
    };

    // Maneja una operación de lectura detectada en el bus (BusRd)
    // Si esta caché tiene el bloque, puede responder indicando que el bloque existe
    // y si debe pasar al estado "Shared"
    SnoopResponse handleBusRead(int address);

    // Maneja una operación de lectura exclusiva (BusRdX)
    // Ocurre cuando otro PE quiere escribir, por lo tanto se invalidan otras copias
    SnoopResponse handleBusReadX(int address);

    // Maneja una operación de invalidación (BusInvalidate)
    // Pone el bloque en estado "Invalid" si esta caché tiene esa dirección
    void handleBusInvalidate(int address);

    // Procesa una lectura local (hecha por este mismo PE)
    // shared_signal: indica si otras cachés también tienen el bloque
    // data_from_source: dato leído (de memoria o de otro PE)
    void processLocalRead(int address, bool shared_signal, int data_from_source);

    // Procesa una escritura local (hecha por este mismo PE)
    // Debe realizar las transiciones de estado correspondientes (por ejemplo S->M o E->M)
    void processLocalWrite(int address, int data);

private:
    // Función auxiliar interna para manejar transiciones de estado en una lectura local
    // Si otras cachés tienen el bloque, pasa a "Shared", si no, pasa a "Exclusive"
    void transitionOnRead(int address, bool other_caches_have);

    // Función auxiliar interna para manejar transiciones de estado en una escritura local
    // Por ejemplo, cambia el estado del bloque a "Modified"
    void transitionOnWrite(int address);
};

#endif
