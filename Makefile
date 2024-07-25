build: glfw raylib sdl pntr

test: clean test-sdl

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

pntr:
	$(MAKE) -C demo/pntr test

web:
	$(MAKE) -C demo/raylib web
