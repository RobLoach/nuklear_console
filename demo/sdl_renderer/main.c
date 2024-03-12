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
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "../../vendor/nuklear/nuklear.h"
#include "../../vendor/nuklear/demo/sdl_renderer/nuklear_sdl_renderer.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#include "../common/nuklear_console_demo.c"

int main(int argc, char *argv[]) {
    /* Platform */
    SDL_Window *win;
    SDL_Renderer *renderer;
    int running = 1;
    int flags = 0;
    float font_scale = 3;

    /* GUI */
    struct nk_context *ctx;

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

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
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        /* GUI */
        if (nk_begin(ctx, "nuklear_console", nk_rect(50, 50, WINDOW_WIDTH - 100, WINDOW_HEIGHT - 100),
            NK_WINDOW_BORDER|NK_WINDOW_TITLE))
        {
            // enum {EASY, HARD};
            // static int op = EASY;
            // static int property = 20;

            // nk_layout_row_static(ctx, 30, 80, 1);
            // if (nk_button_label(ctx, "button"))
            //     fprintf(stdout, "button pressed\n");
            // nk_layout_row_dynamic(ctx, 30, 2);
            // if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            // if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
            // nk_layout_row_dynamic(ctx, 25, 1);
            // nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

            // nk_layout_row_dynamic(ctx, 20, 1);
            // nk_label(ctx, "background:", NK_TEXT_LEFT);
            // nk_layout_row_dynamic(ctx, 25, 1);
            // if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
            //     nk_layout_row_dynamic(ctx, 120, 1);
            //     bg = nk_color_picker(ctx, bg, NK_RGBA);
            //     nk_layout_row_dynamic(ctx, 25, 1);
            //     bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
            //     bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
            //     bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
            //     bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
            //     nk_combo_end(ctx);
            // }

            nuklear_console_demo_render();
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