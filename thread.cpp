#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <vector>
#include <sstream>

std::mutex print_mutex;  // Evita mezcla de texto en consola

class PE {
public:
    void ejecutar(const std::string& op, int a, int b, const std::string& name,
                  std::chrono::steady_clock::time_point start_time) 
    {
        int result = 0;

        if (op == "mul") result = a * b;
        else if (op == "add") result = a + b;
        else if (op == "sub") result = a - b;
        else if (op == "div" && b != 0) result = a / b;
        else {
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << name << " -> operación no válida o división por cero.\n";
            return;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << "[" << elapsed << " ms] "
                  << name << " -> " << op << "(" << a << ", " << b << ") = " << result << std::endl;
    }
};

int main() {
    std::vector<std::string> instrucciones = {
        "PE1 mul 3 4",
        "PE2 mul 1 2",
        "PE3 add 5 7"
    };

    std::vector<std::thread> hilos;
    std::vector<PE> procesadores(instrucciones.size());
    auto start_time = std::chrono::steady_clock::now();

    for (size_t i = 0; i < instrucciones.size(); ++i) {
        std::istringstream ss(instrucciones[i]);
        std::string nombre, operacion;
        int a, b;
        ss >> nombre >> operacion >> a >> b;

        hilos.emplace_back(&PE::ejecutar, &procesadores[i], operacion, a, b, nombre, start_time);
    }

    for (auto& h : hilos)
        h.join();

    return 0;
}
