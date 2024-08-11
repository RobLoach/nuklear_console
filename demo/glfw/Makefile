# Install
BIN = nuklear_console_demo_glfw

# Flags
#CFLAGS += -std=c89 -Wall -Wextra -pedantic -O2
CFLAGS += -std=c99 -Wall -Wextra -pedantic -g -O0

SRC = main.c
OBJ = $(SRC:.c=.o)

ifeq ($(OS),Windows_NT)
BIN := $(BIN).exe
LIBS = -lglfw3 -lopengl32 -lm -lGLU32 -lGLEW32
else
	UNAME_S := $(shell uname -s)
	GLFW3 := $(shell pkg-config --libs glfw3)
	ifeq ($(UNAME_S),Darwin)
		LIBS := $(GLFW3) -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -lm -lGLEW -L/usr/local/lib
	else
		LIBS = $(GLFW3) -lGL -lm -lGLU -lGLEW
	endif
endif

$(BIN): clean
	$(CC) $(SRC) $(CFLAGS) -o $(BIN) $(LIBS)

test: $(BIN)
	./$(BIN)

clean:
	rm -rf $(BIN) $(OBJS)
