#ifndef SNOOP_H
#define SNOOP_H

#include "mesi_state.h"
#include <iomanip>
#include <iostream>
#include <string>

class Cache;
class Interconnect;
class CacheLine;

// Trnsacciones de bus
enum class bus_transaction {
  READ,       // Petición de lectura
  WRITE,      // Petición de escritura
  INVALIDATE, // Invalidar otras copias
  FLUSH,      // Escribir de vuelta a memoria
  NONE
};

// Mensaje del bus
struct bus_message {
  int pe_id;
  bus_transaction transaction;
  int address;
  double data;

  bus_message(int id, bus_transaction trans, int addr, double d = 0.0)
      : pe_id(id), transaction(trans), address(addr), data(d) {}
};

// Clase del Módulo Snoop
class SnoopModule {
public:
  // Estructura de respuesta del snoop cuando hay peticiones del bus
  struct SnoopResponse {
    bool hit;       // Tenemos el dato
    bool shared;    // El dato está compartido
    bool modified;  // El dato fue modificado
    double data[4]; // Datos del bloque completo

    SnoopResponse() : hit(false), shared(false), modified(false) {
      for (int i = 0; i < 4; i++)
        data[i] = 0.0;
    }
  };

private:
  int pe_id;                  // ID del PE
  Cache *cache;               // Puntero a la caché local
  Interconnect *interconnect; // Puntero al interconnect

  void
  transitionOnRead(int address,
                   bool other_caches_have); // Metodo para decidir entre los
                                            // estado S y E en caso de lectura
  void transitionOnWrite(int address);      // Realiza cambio al estado M

  // Calcula los campos de dirección igual que Cache, para poder indicar la
  // línea
  void calculateAddressFields(int addr, int &word_index, int &offset,
                              int &block_number, int &index, int &tag);

public:
  // Constructor
  SnoopModule(int id, Cache *c, Interconnect *ic = nullptr);

  // Cambios externos (Otros PE's)
  // Otro PE quiere leer
  SnoopResponse handleBusRead(int address);

  // Invalida la línea si la tengo
  void handleBusInvalidate(int address);

  // Operaciones con el Interconnect
  // Maneja un Read Miss (solicita el dato al interconnect, lo recibe y
  // actualiza caché)
  CacheLine handleReadMiss(int address);

  // Maneja un wirte, hit o miss, (si es miss, trae el dato, invalida las copias
  // y actiliza caché)
  CacheLine handleWrite(int address, double value);

  // Get del ID
  int getPEId() const { return pe_id; }

  // Setter para interconnect (solo en caso de necesitar)
  void setInterconnect(Interconnect *ic) { interconnect = ic; }
};

#endif
