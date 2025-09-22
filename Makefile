# Performance Monitor Makefile

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
DEBUG_FLAGS = -g -DDEBUG -O0

# Directories
SRC_DIR = src
BUILD_DIR = build
TARGET = monitor
DEMO_TARGET = microservice_demo

# Source files
MONITOR_SOURCES = $(SRC_DIR)/monitor.cpp $(SRC_DIR)/main.cpp
DEMO_SOURCES = $(SRC_DIR)/monitor.cpp $(SRC_DIR)/mock_service.cpp $(SRC_DIR)/microservice_demo.cpp

MONITOR_OBJECTS = $(MONITOR_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEMO_OBJECTS = $(DEMO_SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Default target - build both
all: $(TARGET) $(DEMO_TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link executables
$(TARGET): $(MONITOR_OBJECTS)
	$(CXX) $(MONITOR_OBJECTS) -o $(TARGET) -pthread

$(DEMO_TARGET): $(DEMO_OBJECTS)
	$(CXX) $(DEMO_OBJECTS) -o $(DEMO_TARGET) -pthread

# Debug builds
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: clean all

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(DEMO_TARGET)

# Install basic monitor
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

# Run the basic HTTP monitor
run: $(TARGET)
	./$(TARGET)

# Run the full microservice demo
demo: $(DEMO_TARGET)
	./$(DEMO_TARGET)

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build both monitor and demo (default)"
	@echo "  monitor  - Build basic HTTP monitor only"
	@echo "  demo     - Build and run microservice demo"
	@echo "  run      - Build and run basic monitor"
	@echo "  debug    - Build with debug flags"
	@echo "  clean    - Remove build files"
	@echo "  install  - Install monitor to system"
	@echo "  help     - Show this help"

# Individual targets
monitor: $(TARGET)

# Phony targets
.PHONY: all debug clean install uninstall run demo help monitor