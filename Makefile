build: glfw raylib sdl

test: clean test-sdl

clean:
	$(MAKE) -C demo/sdl_renderer clean
	$(MAKE) -C demo/raylib clean
	$(MAKE) -C demo/glfw clean
	rm -rf build

raylib:
	$(MAKE) -C demo/raylib

glfw:
	$(MAKE) -C demo/glfw

sdl:
	$(MAKE) -C demo/sdl

test-sdl:
	$(MAKE) -C demo/sdl_renderer test
