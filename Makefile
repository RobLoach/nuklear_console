build: glfw raylib sdl pntr

test: clean test-sdl

clean:
	$(MAKE) -C demo/sdl_renderer clean
	$(MAKE) -C demo/raylib clean
	$(MAKE) -C demo/glfw clean
	$(MAKE) -C demo/pntr clean
	rm -rf build

raylib: submodules
	$(MAKE) -C demo/raylib test

glfw: submodules
	$(MAKE) -C demo/glfw test

sdl: submodules
	$(MAKE) -C demo/sdl_renderer test

pntr: submodules
	$(MAKE) -C demo/pntr test

web: submodules
	$(MAKE) -C demo/raylib web

vendor/Nuklear/nuklear.h:
	git submodule update --init --recursive

submodules: vendor/Nuklear/nuklear.h
