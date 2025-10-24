Computer Architecture II

Project 1: Modeling of a Multiprocessor (MP) System with MESI Cache Coherence for Parallel Dot Product Computation

This project is a simulator of the MESI cache coherence protocol, implemented in C++ with a graphical interface developed using FLTK.
It allows the visualization of the behavior of four processing elements, each with its own cache memory and Snoop module.

------------------------------------------------------------
System Requirements

Dependencies:
- Compiler: g++ (version 9.0 or higher)
- Library: FLTK
- Tool: make

Installing FLTK (Linux):
sudo apt-get install libfltk1.3-dev

------------------------------------------------------------
Compilation and Execution

Using Makefile:
make
make run
make clean

Without Makefile (alternative)
g++ pe_interfaz.cpp file_line_selector.cpp mem.cpp interconnect.cpp processing_element.cpp cache.cpp snoop.cpp -o pe_interfaz -lfltk
./pe_interfaz
./pe_interfaz

Validate value
cd v1/test/
g++ validar.cpp -o validar
./validar



------------------------------------------------------------
Authors

Developed by:
- Jose Andrés Arroyo Meoño
- Carolina Rodriguez Hall
- Noemí Vargas Soto
Computer Engineering Students
Costa Rica Institute of Technology (ITCR)

