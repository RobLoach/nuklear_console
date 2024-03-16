#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include <SDL2/SDL.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "../../vendor/nuklear/nuklear.h"
#include "../../vendor/nuklear/demo/sdl_renderer/nuklear_sdl_renderer.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

nk_bool showWindowTitle;

#include "../common/nuklear_console_demo.c"

SDL_GameController *findController() {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    /* Platform */
    SDL_Window *win;
    SDL_Renderer *renderer;
    int running = 1;
    int flags = 0;
    float font_scale = 3;
    showWindowTitle = nk_true;

    /* GUI */
    struct nk_context *ctx;

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);

    SDL_GameController *controller = findController();

    win = SDL_CreateWindow("nuklear_console_demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN|SDL_WINDOW_ALLOW_HIGHDPI);

    if (win == NULL) {
        SDL_Log("Error SDL_CreateWindow %s", SDL_GetError());
        return 1;
    }

    flags |= SDL_RENDERER_ACCELERATED;
    flags |= SDL_RENDERER_PRESENTVSYNC;

#if 0
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
#endif

    renderer = SDL_CreateRenderer(win, -1, flags);

    if (renderer == NULL) {
        SDL_Log("Error SDL_CreateRenderer %s", SDL_GetError());
        return 1;
    }

    /* GUI */
    ctx = nk_sdl_init(win, renderer);
    {
        struct nk_font_atlas *atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font *font;

        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        nk_sdl_font_stash_end();
        nk_style_set_font(ctx, &font->handle);
    }

    nuklear_console_demo_init(ctx);

    while (running) {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) goto cleanup;
            if (evt.type == SDL_KEYUP && evt.key.keysym.scancode == SDL_SCANCODE_ESCAPE) running = 0;

            // Mock SDL game controller to nuklear keyboard.
            if (evt.type == SDL_CONTROLLERBUTTONDOWN || evt.type == SDL_CONTROLLERBUTTONUP) {
                if (evt.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller))) {
                    switch (evt.cbutton.button) {
                        case SDL_CONTROLLER_BUTTON_DPAD_UP:
                            nk_input_key(ctx, NK_KEY_UP, evt.type == SDL_CONTROLLERBUTTONDOWN);
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                            nk_input_key(ctx, NK_KEY_DOWN, evt.type == SDL_CONTROLLERBUTTONDOWN);
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                            nk_input_key(ctx, NK_KEY_LEFT, evt.type == SDL_CONTROLLERBUTTONDOWN);
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                            nk_input_key(ctx, NK_KEY_RIGHT, evt.type == SDL_CONTROLLERBUTTONDOWN);
                            break;
                        case SDL_CONTROLLER_BUTTON_B:
                            nk_input_key(ctx, NK_KEY_BACKSPACE, evt.type == SDL_CONTROLLERBUTTONDOWN);
                            break;
                        case SDL_CONTROLLER_BUTTON_A:
                            nk_input_key(ctx, NK_KEY_ENTER, evt.type == SDL_CONTROLLERBUTTONDOWN);
                            break;
                    }
                }
            }

            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        int flags = NK_WINDOW_BORDER;
        flags |= NK_WINDOW_SCROLL_AUTO_HIDE;
        if (showWindowTitle) {
            flags |= NK_WINDOW_TITLE;
        }

        /* GUI */
        if (nk_begin(ctx, "nuklear_console", nk_rect(50, 50, WINDOW_WIDTH - 100, WINDOW_HEIGHT - 100), flags)) {
            /* Render it, and see if we're to stop running. */
            if (nuklear_console_demo_render()) {
                running = 0;
            }
        }
        nk_end(ctx);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        nk_sdl_render(NK_ANTI_ALIASING_ON);

        SDL_RenderPresent(renderer);
    }

cleanup:
    nk_sdl_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
