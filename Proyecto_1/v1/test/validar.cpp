#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  int N;
  cout << "Ingrese el tamaño de los vectores (N): ";
  cin >> N;

  ifstream archivo("memoria.txt");
  if (!archivo.is_open()) {
    cerr << "Error: no se pudo abrir el archivo memoria.txt" << endl;
    return 1;
  }

  vector<double> memoria; // guardará todos los datos leídos
  double valor;

  // Leer todos los números del archivo
  while (archivo >> valor) {
    memoria.push_back(valor);
  }
  archivo.close();

  // Crear los dos vectores con tamaño máximo N
  vector<double> vector1;
  vector<double> vector2;

  // Llenar el primer vector
  for (int i = 0; i < memoria.size() && i < N; ++i) {
    vector1.push_back(memoria[i]);
  }

  // Llenar el segundo vector
  for (int i = N; i < memoria.size() && i < 2 * N; ++i) {
    vector2.push_back(memoria[i]);
  }

  // Mostrar resultados
  cout << "\nVector 1:\n";
  for (double x : vector1)
    cout << x << " ";
  cout << "\n\nVector 2:\n";
  for (double x : vector2)
    cout << x << " ";
  cout << endl;

  // Calcular el producto punto (si ambos tienen el mismo tamaño)
  double producto_punto = 0.0;
  int min_tam = min(vector1.size(), vector2.size());

  for (int i = 0; i < min_tam; ++i) {
    producto_punto += vector1[i] * vector2[i];
  }

  cout << "\nProducto punto: " << producto_punto << endl;

  return 0;
}
