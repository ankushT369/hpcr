CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g
LDFLAGS = -pthread
LDLIBS = -lboost_system -lboost_thread -lglog -lyaml-cpp

# Source files
SERVER_SRC = hpcr.cpp
CLIENT_SRC = client.cpp

# Build directory
BUILD_DIR = build

# Object files (in build directory)
SERVER_OBJ = $(BUILD_DIR)/$(SERVER_SRC:.cpp=.o)
CLIENT_OBJ = $(BUILD_DIR)/$(CLIENT_SRC:.cpp=.o)

# Targets
all: $(BUILD_DIR)/hpcr $(BUILD_DIR)/clientApp

# Server binary
$(BUILD_DIR)/hpcr: $(SERVER_OBJ)
	$(CXX) $(LDFLAGS) $(SERVER_OBJ) $(LDLIBS) -o $@

# Client binary
$(BUILD_DIR)/clientApp: $(CLIENT_OBJ)
	$(CXX) $(LDFLAGS) $(CLIENT_OBJ) $(LDLIBS) -o $@

# Compile .cpp to .o in build directory
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean
clean:
	rm -rf $(BUILD_DIR)
