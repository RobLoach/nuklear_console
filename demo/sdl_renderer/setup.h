#include <string.h>
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
#include "../../vendor/Nuklear/nuklear.h"
#include "../../vendor/Nuklear/demo/sdl_renderer/nuklear_sdl_renderer.h"

#include "../common/nuklear_console_demo.c"

struct demo_ctx {
    SDL_Renderer* renderer;
    struct nk_context* ctx;
    struct nk_console* console;
    struct nk_image img;
    SDL_Texture* _texture;
    SDL_Window* _window;
    int window_width;
    int window_height;
};

static int configure(struct demo_ctx* demo) {
    /* Platform */
    int flags = 0;
    float font_scale = 3;

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);

    demo->_window = SDL_CreateWindow("nuklear_console_demo_multiple_window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, demo->window_width, demo->window_height, SDL_WINDOW_SHOWN);

    if (demo->_window == NULL) {
        SDL_Log("Error SDL_CreateWindow %s", SDL_GetError());
        return 1;
    }

    demo->renderer = SDL_CreateRenderer(demo->_window, -1, flags);

    if (demo->renderer == NULL) {
        SDL_Log("Error SDL_CreateRenderer %s", SDL_GetError());
        return 1;
    }

    /* GUI */
    demo->ctx = nk_sdl_init(demo->_window, demo->renderer);
    {
        struct nk_font_atlas* atlas;
        struct nk_font_config config = nk_font_config(0);
        struct nk_font* font;

        nk_sdl_font_stash_begin(&atlas);
        font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
        nk_sdl_font_stash_end();
        nk_style_set_font(demo->ctx, &font->handle);
    }

    // Attempt to load the sample image.
    demo->img = nk_image_id(0);
    SDL_Surface* surface = SDL_LoadBMP("image.bmp");
    if (surface != NULL) {
        demo->_texture = SDL_CreateTextureFromSurface(demo->renderer, surface);
        if (demo->_texture != NULL) {
            demo->img = nk_image_ptr(demo->_texture);
            demo->img.w = surface->w;
            demo->img.h = surface->h;
            demo->img.region[0] = 0;
            demo->img.region[1] = 0;
            demo->img.region[2] = surface->w;
            demo->img.region[3] = surface->h;
        }
        SDL_FreeSurface(surface);
    }

    return 0;
}

static struct demo_ctx init_demo_context(int window_width, int window_height) {
    struct demo_ctx demo;
    demo.window_width = window_width;
    demo.window_height = window_height;
    configure(&demo);
    return demo;
}

static int cleanup(struct demo_ctx* demo) {
    if (demo->_texture != NULL) {
        SDL_DestroyTexture(demo->_texture);
    }
    nuklear_console_demo_free(demo->console);
    nk_sdl_shutdown();
    SDL_DestroyRenderer(demo->renderer);
    SDL_DestroyWindow(demo->_window);
    SDL_Quit();

    return 0;
}

static void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    nk_sdl_render(NK_ANTI_ALIASING_ON);

    SDL_RenderPresent(renderer);
}
