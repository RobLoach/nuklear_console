.PHONY: build test clean raylib glfw sdl sdl3 pntr web clang

build: glfw raylib sdl pntr

test:
	cmake -B build -S .
	cmake --build build
	ctest --test-dir build --output-on-failure

clean:
	$(MAKE) -C demo/sdl_renderer clean
	$(MAKE) -C demo/raylib clean
	$(MAKE) -C demo/glfw clean
	$(MAKE) -C demo/pntr clean
	rm -rf build

raylib:
	$(MAKE) -C demo/raylib test

glfw:
	$(MAKE) -C demo/glfw test

sdl:
	$(MAKE) -C demo/sdl_renderer test

sdl3:
	$(MAKE) -C demo/sdl3_renderer test

pntr:
	$(MAKE) -C demo/pntr test

web:
	$(MAKE) -C demo/raylib web

clang:
	clang-format -i *.h
