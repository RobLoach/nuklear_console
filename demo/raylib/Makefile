build/Makefile:
	cmake -B build -S .

compile: build/Makefile
	cmake --build build

test: compile
	./build/nuklear_console_raylib
