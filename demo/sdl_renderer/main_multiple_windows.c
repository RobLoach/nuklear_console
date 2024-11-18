#include "setup.h"

int main(int argc, char* argv[]) {
    NK_UNUSED(argc);
    NK_UNUSED(argv);

    struct demo_ctx demo = init_demo_context(1280, 720);

    struct nk_context* ctx = demo.ctx;

    // Init console
    demo.console = nk_console_init(ctx);

    const int WINDOW_COUNT = 3;

    int window_flags = NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_TITLE;

    // Register console windows
    struct demo_console_state states[WINDOW_COUNT];
    for (int i = 0; i < WINDOW_COUNT; i++) {
        states[i] = demo_console_state_defaults();
        snprintf(states[i].title, sizeof(states[i].title), "nuklear_console_%d", i + 1);
        nk_console* window = nk_console_window(demo.console, states[i].title, nk_rect(i * demo.window_width / WINDOW_COUNT, 0, demo.window_width / WINDOW_COUNT, demo.window_height), window_flags);

        nuklear_console_demo_init(ctx, window, &states[i], NULL, demo.img);
    }

    int running = 1;

    while (running) {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        nk_gamepad_update(nk_console_get_gamepads(demo.console));
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) return cleanup(&demo);
            if (evt.type == SDL_KEYUP && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) running = 0;

            nk_sdl_handle_event(&evt);
            nk_gamepad_sdl_handle_event(nk_console_get_gamepads(demo.console), &evt);
        }
        nk_input_end(ctx);

        /* GUI */
        if (nuklear_console_demo_render(demo.console)) {
            running = 0;
        }

        render(demo.renderer);
    }

    return 0;
}
