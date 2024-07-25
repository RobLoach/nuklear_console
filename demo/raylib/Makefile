compile: build/Makefile
	cmake --build build

build/Makefile:
	cmake -B build -S .

test: compile
	./build/nuklear_console_demo_raylib

clean:
	rm -rf build

web:
	emcmake cmake -B build -S . -DPLATFORM=Web
	emmake make -C build
