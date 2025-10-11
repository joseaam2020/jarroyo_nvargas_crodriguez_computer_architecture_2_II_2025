# Compilador y flags
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

# Directorios
SRC_DIR := .
BUILD_DIR := build

# Archivos fuente y objetos
SRCS := $(SRC_DIR)/main.cpp \
        $(SRC_DIR)/cache.cpp \
        $(SRC_DIR)/interconnect.cpp \
        $(SRC_DIR)/mem.cpp \
        $(SRC_DIR)/processing_element.cpp \
        $(SRC_DIR)/snoop.cpp

OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Ejecutable final
TARGET := $(BUILD_DIR)/mesi_simulator

# Regla por defecto
all: $(TARGET)

# Enlazado final
$(TARGET): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilación de objetos
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos
clean:
	rm -rf $(BUILD_DIR)

# Ejecutar
run: $(TARGET)
	./$(TARGET)
