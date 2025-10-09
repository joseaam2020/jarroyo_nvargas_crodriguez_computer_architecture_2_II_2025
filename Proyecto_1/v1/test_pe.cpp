#include "processing_element.h"

int main() {
  // Crear un PE con ID 0
  ProcessingElement pe(0);

  std::cout << "===== INICIO DE PRUEBAS =====\n\n";

  // Simular escritura de valores en registros directamente (solo para test)

  // Aumentar valores en R0 y R1 usando inc
  pe.inc(0); // R0 = 1.0
  pe.inc(0); // R0 = 2.0
  pe.inc(1); // R1 = 1.0
  pe.inc(1); // R1 = 2.0
  pe.inc(1); // R1 = 3.0

  // Guardar R0 y R1 en memoria
  pe.store(0, 0); // mem[0] = 2.0
  pe.store(1, 8); // mem[1] = 3.0

  // Multiplicar: R2 = R0 * R1 = 6.0
  pe.fmul(2, 0, 1);

  // Sumar: R3 = R0 + R1 = 5.0
  pe.fadd(3, 0, 1);

  // Guardar resultados en memoria
  pe.store(2, 16); // mem[2] = 6.0
  pe.store(3, 24); // mem[3] = 5.0

  pe.dec(2);      // R2 = 5.0
  pe.load(3, 16); // R3 = 6.0

  pe.fmul(4, 2, 3); // R4 = 5.0 * 6.0 = 30.0

  pe.store(3, 32); // mem[3] = 5.0

  // Mostrar estado final del PE y su cache
  pe.printStatus();

  std::cout << "\n===== FIN DE PRUEBAS =====\n";
  return 0;
}
