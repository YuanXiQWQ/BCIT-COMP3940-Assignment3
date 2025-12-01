CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -pthread -Iserver/include
LDFLAGS := -pthread

BIN_DIR := bin
BUILD_DIR := build
SERVER_BUILD_DIR := $(BUILD_DIR)/server
CLIENT_BUILD_DIR := $(BUILD_DIR)/client

SERVER_SOURCES := $(wildcard server/src/*.cpp)
SERVER_OBJECTS := $(patsubst server/src/%.cpp,$(SERVER_BUILD_DIR)/%.o,$(SERVER_SOURCES))
SERVER_TARGET := $(BIN_DIR)/upload_server

CLIENT_SOURCES := $(wildcard client/src/*.cpp)
CLIENT_OBJECTS := $(patsubst client/src/%.cpp,$(CLIENT_BUILD_DIR)/%.o,$(CLIENT_SOURCES))
CLIENT_TARGET := $(BIN_DIR)/upload_client

.PHONY: all server client clean

all: server client

server: $(SERVER_TARGET)

client: $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(SERVER_BUILD_DIR)/%.o: server/src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CLIENT_BUILD_DIR)/%.o: client/src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
