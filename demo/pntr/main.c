#define PNTR_APP_IMPLEMENTATION
#define PNTR_ENABLE_DEFAULT_FONT
#define PNTR_ENABLE_MATH
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "pntr_app.h"

#define PNTR_NUKLEAR_IMPLEMENTATION
#include "pntr_nuklear.h"

#define NK_GAMEPAD_PNTR
#define NK_GAMEPAD_IMPLEMENTATION
#include "nuklear_gamepad.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300

#define NK_CONSOLE_ENABLE_TINYDIR
#include "../common/nuklear_console_demo.c"

typedef struct AppData {
    pntr_font* font;
    struct nk_context* ctx;
    pntr_image* image;
    struct nk_console* console;
} AppData;

bool Init(pntr_app* app) {
    AppData* appData = pntr_load_memory(sizeof(AppData));
    pntr_app_set_userdata(app, appData);

    // Load the default font
    appData->font = pntr_load_font_default();
    appData->ctx = pntr_load_nuklear(appData->font);
    appData->image = pntr_load_image("resources/image.png");

    // Initialize the Gamepads
    appData->console = nuklear_console_demo_init(appData->ctx, app, pntr_image_nk(appData->image));

    return true;
}

bool Update(pntr_app* app, pntr_image* screen) {
    AppData* appData = (AppData*)pntr_app_userdata(app);
    struct nk_context* ctx = appData->ctx;

    // Update the pntr input state.
    pntr_nuklear_update(ctx, app);

    // Update the gamepad state
    nk_gamepad_update(nk_console_get_gamepads(console));

    // Clear the background
    pntr_clear_background(screen, PNTR_BLACK);

    /* GUI */
    int flags = NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE;
    if (nk_begin(ctx, "nuklear_console", nk_rect(0, 0, screen->width, screen->height), flags)) {
        /* Render it, and see if we're to stop running. */
        if (nuklear_console_demo_render()) {
            nk_end(ctx);
            return false;
        }
    }
    nk_end(ctx);

    // Draw it on the screen
    pntr_draw_nuklear(screen, ctx);

    return true;
}

void Close(pntr_app* app) {
    AppData* appData = (AppData*)pntr_app_userdata(app);

    nuklear_console_demo_free();

    // Unload the font
    pntr_unload_font(appData->font);
    pntr_unload_image(appData->image);
    pntr_unload_nuklear(appData->ctx);

    pntr_unload_memory(appData);
}

pntr_app Main(int argc, char* argv[]) {
    return (pntr_app) {
        .width = WINDOW_WIDTH,
        .height = WINDOW_HEIGHT,
        .title = "nuklear_console_demo_pntr",
        .init = Init,
        .update = Update,
        .close = Close,
        .fps = 60
    };
}
