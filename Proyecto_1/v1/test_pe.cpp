#include "processing_element.h"

int main() {
  // Crear un PE con ID 0
  ProcessingElement pe(0);

  std::cout << "===== INICIO DE PRUEBAS =====\n\n";

  pe.mov(0, 16);    // mov r0, #16
  pe.mov(1, 2);     // mov r1, #2
  pe.mov(2, 3);     // mov r2, #3
  pe.fmul(3, 1, 2); // fmul r3, r1, r2
  pe.store(3, 0);   // stor r3, [r0] → mem[16] = r3
  pe.mov(4, 6);     // mov r4, #6
  pe.load(5, 0);    // load r5, [r0] → r5 = mem[16]
  pe.fadd(6, 5, 4); // fadd r6, r5, r4
  pe.printStatus();
  std::cout << "\n===== FIN DE PRUEBAS =====\n";
  return 0;
}
