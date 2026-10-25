CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS += -Isrc

.PHONY: all
all: build/server

build:
	mkdir -p build

build/server: src/server.cpp src/core.cpp src/core.h src/net.h | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) src/server.cpp src/core.cpp -o $@
