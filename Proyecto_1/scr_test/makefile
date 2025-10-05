CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

TARGET = mesi_simulator

SOURCES = main.cpp \
          mesi_protocol.cpp \
          cache.cpp \
          snoop.cpp \
          interconnect.cpp \
          memory.cpp \
          processing_element.cpp

OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
