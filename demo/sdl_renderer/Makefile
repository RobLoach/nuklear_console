# Install
BIN_MAIN = nuklear_console_demo_sdl
BIN_MULTIPLE = nuklear_console_demo_sdl_multiple_windows

# Flags

# Use C++
CFLAGS += -std=c99 -pedantic -O0
CXXFLAGS += -pedantic -O0 -fpermissive -std=c++17

CFLAGS += `sdl2-config --cflags`
CXXFLAGS += `sdl2-config --cflags`

SRC_MAIN = main.c
SRC_MULTIPLE = main_multiple_windows.c

OBJ_MAIN = $(SRC_MAIN:.c=.o)
OBJ_MULTIPLE = $(SRC_MULTIPLE:.c=.o)

ifeq ($(OS),Windows_NT)
#TODO
#BIN := $(BIN).exe
#LIBS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lm -lGLU32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LIBS += -lm -ldl `sdl2-config --libs`
	else
		LIBS += -lm -ldl `sdl2-config --libs`
	endif
endif

# Default target: build both binaries
all: $(BIN_MAIN) $(BIN_MULTIPLE)

# Rule for nuklear_console_demo_sdl (main.c)
$(BIN_MAIN): $(SRC_MAIN)
	$(CC) $(SRC_MAIN) $(CFLAGS) -o $(BIN_MAIN) $(LIBS)

# Rule for nuklear_console_demo_sdl_multiple (main_multiple.c)
$(BIN_MULTIPLE): $(SRC_MULTIPLE)
	$(CC) $(SRC_MULTIPLE) $(CFLAGS) -o $(BIN_MULTIPLE) $(LIBS)

# Clean up
clean:
	rm -rf $(BIN_MAIN) $(BIN_MULTIPLE) $(OBJ_MAIN) $(OBJ_MULTIPLE)

# Test the main binary
test: $(BIN_MAIN)
	./$(BIN_MAIN)
