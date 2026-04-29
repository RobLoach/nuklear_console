#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#include <SDL3/SDL.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_COMMAND_USERDATA
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_IMPLEMENTATION
#include "../../vendor/Nuklear/nuklear.h"

#define NK_SDL3_RENDERER_IMPLEMENTATION
#include "../../vendor/Nuklear/demo/sdl3_renderer/nuklear_sdl3_renderer.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Set up the Nuklear Console Demo
#include "../common/nuklear_console_demo.c"
#include "../../nuklear_console_sdl.h"

struct nk_sdl3_app {
    SDL_Window *window;
    SDL_Renderer *renderer;
    struct nk_context *ctx;
    float font_scale;
    SDL_Texture *texture;
    int running;

    // Nuklear Console
    struct nk_console *console;
};

static SDL_AppResult app_fail(void) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
    NK_UNUSED(argc);
    NK_UNUSED(argv);

    struct nk_sdl3_app *app = SDL_malloc(sizeof(*app));
    if (!app) {
        return app_fail();
    }
    SDL_memset(app, 0, sizeof(*app));
    app->running = 1;
    *appstate = app;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD)) {
        return app_fail();
    }

    if (!SDL_CreateWindowAndRenderer("nuklear_console_demo_sdl3",
            WINDOW_WIDTH, WINDOW_HEIGHT,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY,
            &app->window, &app->renderer)) {
        return app_fail();
    }

    SDL_SetRenderVSync(app->renderer, 1);

    {
        const float scale = SDL_GetWindowDisplayScale(app->window);
        SDL_SetRenderScale(app->renderer, scale, scale);
        app->font_scale = scale;
    }

    // Set up the SDL3 Nuklear Context.
    app->ctx = nk_sdl_init(app->window, app->renderer, nk_sdl_allocator());

    // Font Setup
    {
        struct nk_font_config config = nk_font_config(0);
        struct nk_font_atlas* atlas = nk_sdl_font_stash_begin(app->ctx);
        struct nk_font* font = nk_font_atlas_add_default(atlas, 32 * app->font_scale, &config);
        nk_sdl_font_stash_end(app->ctx);

        font->handle.height /= app->font_scale;
        nk_style_set_font(app->ctx, &font->handle);
    }

    // Set up the image
    struct nk_image img = nk_image_id(0);
    SDL_Surface *surface = SDL_LoadBMP("image.bmp");
    if (surface != NULL) {
        app->texture = SDL_CreateTextureFromSurface(app->renderer, surface);
        if (app->texture != NULL) {
            img = nk_image_ptr(app->texture);
            img.w = surface->w;
            img.h = surface->h;
            img.region[0] = 0;
            img.region[1] = 0;
            img.region[2] = surface->w;
            img.region[3] = surface->h;
        }
        SDL_DestroySurface(surface);
    }

    // Create the Nuklear Console Demo
    app->console = nuklear_console_demo_init(app->ctx, NULL, img);

    nk_input_begin(app->ctx);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    struct nk_sdl3_app *app = (struct nk_sdl3_app *)appstate;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
        case SDL_EVENT_KEY_UP:
            // Quit when pressing the escape key
            if (event->key.scancode == SDL_SCANCODE_ESCAPE)
                return SDL_APP_SUCCESS;
            break;
    }

    SDL_ConvertEventToRenderCoordinates(app->renderer, event);

    nk_sdl_handle_event(app->ctx, event);
    nk_gamepad_sdl3_handle_event(nk_console_get_gamepads(app->console), event);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    struct nk_sdl3_app *app = (struct nk_sdl3_app *)appstate;
    struct nk_context *ctx = app->ctx;

    nk_input_end(ctx);

    nk_gamepad_update(nk_console_get_gamepads(app->console));

    int w, h;
    SDL_GetWindowSize(app->window, &w, &h);
    // Scale logical size back to window coords
    float lw = (float)w / SDL_GetWindowDisplayScale(app->window);
    float lh = (float)h / SDL_GetWindowDisplayScale(app->window);

    int flags = NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_TITLE;
    if (nk_begin(ctx, "nuklear_console", nk_rect(0, 0, (float)lw, (float)lh), flags)) {
        if (nuklear_console_demo_render()) {
            SDL_DestroyTexture(app->texture);
            app->texture = NULL;
            nuklear_console_demo_free();
            nk_sdl_shutdown(ctx);
            SDL_DestroyRenderer(app->renderer);
            SDL_DestroyWindow(app->window);
            SDL_free(app);
            SDL_Quit();
            exit(0);
        }
    }
    nk_end(ctx);

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);

    nk_sdl_render(ctx, NK_ANTI_ALIASING_ON);
    nk_console_sdl_update_text_input(app->console, app->window);

    SDL_RenderPresent(app->renderer);

    nk_input_begin(ctx);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    NK_UNUSED(result);
    struct nk_sdl3_app *app = (struct nk_sdl3_app *)appstate;
    if (app) {
        nk_input_end(app->ctx);
        if (app->texture) {
            SDL_DestroyTexture(app->texture);
        }
        nuklear_console_demo_free();
        nk_sdl_shutdown(app->ctx);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        SDL_free(app);
    }
    SDL_Quit();
}
