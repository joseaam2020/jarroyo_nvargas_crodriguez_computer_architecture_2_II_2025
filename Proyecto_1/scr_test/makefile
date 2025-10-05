CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = mesi_simulator
BUILD_DIR = build
SOURCES_DIR = .

SOURCES = main.cpp \
          mesi_protocol.cpp \
          cache.cpp \
          snoop.cpp \
          interconnect.cpp \
          memory.cpp \
          processing_element.cpp

# Colocar objetos en build y mantener estructura de carpetas si es necesario
OBJECTS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)

# Crear estructura de directorios necesaria
$(shell mkdir -p $(BUILD_DIR))

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

# Regla para compilar cada archivo .cpp a .o en build/
$(BUILD_DIR)/%.o: $(SOURCES_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: $(BUILD_DIR)/$(TARGET)
	./$(BUILD_DIR)/$(TARGET)

.PHONY: all clean run