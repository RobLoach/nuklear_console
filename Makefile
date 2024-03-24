build: demo/sdl_renderer/bin/demo

test: clean build
	./demo/sdl_renderer/bin/demo

demo/sdl_renderer/bin/demo:
	$(MAKE) -C demo/sdl_renderer

clean:
	rm -rf demo/sdl_renderer/bin

raylib:
	$(MAKE) -C demo/raylib compile
