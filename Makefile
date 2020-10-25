SRC = $(wildcard src/*.cpp)
INCLUDE_DIR = include
CXXFLAGS = -Wall -Werror -Wpedantic -Wextra -std=c++17 -g
LDFLAGS = -lSDL2

all: $(SRC)
	g++ $(SRC) -I$(INCLUDE_DIR) $(LDFLAGS) $(CXXFLAGS) -o chip8
