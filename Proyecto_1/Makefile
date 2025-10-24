# Compilador
CXX = g++

# Directorios
SRC_DIR := .
BUILD_DIR := build

# Flags
CXXFLAGS = -Wall -std=c++17
LDFLAGS = -lfltk

# Archivos fuente
SRC = pe_interfaz.cpp file_line_selector.cpp mem.cpp interconnect.cpp processing_element.cpp cache.cpp snoop.cpp

# Archivos objeto (en build/)
OBJ = $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(SRC))

# Nombre del ejecutable
TARGET = $(BUILD_DIR)/pe_interfaz

# Regla principal
all: $(TARGET)

# Cómo generar el ejecutable
$(TARGET): $(OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Cómo compilar cada .cpp en .o dentro de build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos compilados
clean:
	rm -rf $(BUILD_DIR)

# Para ejecutar el programa con "make run"
run: $(TARGET)
	./$(TARGET)


# # Compilador y flags
# CXX := g++
# CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# # Directorios
# SRC_DIR := .
# BUILD_DIR := build

# # Archivos fuente y objetos
# SRCS := $(SRC_DIR)/main.cpp \
#         $(SRC_DIR)/cache.cpp \
#         $(SRC_DIR)/interconnect.cpp \
#         $(SRC_DIR)/mem.cpp \
#         $(SRC_DIR)/processing_element.cpp \
#         $(SRC_DIR)/snoop.cpp

# OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# # Ejecutable final
# TARGET := $(BUILD_DIR)/mesi_simulator

# # Regla por defecto
# all: $(TARGET)

# # Enlazado final
# $(TARGET): $(OBJS)
# 	@mkdir -p $(BUILD_DIR)
# 	$(CXX) $(CXXFLAGS) -o $@ $^

# # Compilación de objetos
# $(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
# 	@mkdir -p $(BUILD_DIR)
# 	$(CXX) $(CXXFLAGS) -c $< -o $@

# # Limpiar archivos
# clean:
# 	rm -rf $(BUILD_DIR)

# # Ejecutar
# run: $(TARGET)
# 	./$(TARGET)
