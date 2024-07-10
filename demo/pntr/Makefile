compile: build/Makefile
	cmake --build build

build/Makefile:
	cmake -B build -S .

test: compile
	./build/nuklear_console_demo_pntr_raylib

clean:
	rm -rf build
