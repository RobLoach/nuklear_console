BIN = nuklear_console_demo_sdl3

CFLAGS += -std=c11 -pedantic -O0 -g
CFLAGS += $(shell pkg-config --cflags sdl3)

SRC = main.c

ifeq ($(OS),Windows_NT)
	LIBS = $(shell pkg-config --libs sdl3) -lm
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LIBS = $(shell pkg-config --libs sdl3) -lm
	else
		LIBS = $(shell pkg-config --libs sdl3) -lm -ldl
	endif
endif

$(BIN): clean
	$(CC) $(SRC) $(CFLAGS) -o $(BIN) $(LIBS)

clean:
	rm -rf $(BIN)

test: $(BIN)
	./$(BIN)
