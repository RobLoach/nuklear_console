build: demo/sdl_renderer/bin/demo	

test: clean build
	./demo/sdl_renderer/bin/demo

demo/sdl_renderer/bin/demo:
	make -C demo/sdl_renderer

clean:
	rm -rf demo/sdl_renderer/bin
