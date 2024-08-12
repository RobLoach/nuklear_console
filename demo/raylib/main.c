#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#include "raylib.h"

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define RAYLIB_NUKLEAR_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "raylib-nuklear.h"

#include "../common/nuklear_console_demo.c"

void UpdateDrawFrame(void);

struct nk_context *ctx;
nk_bool closeWindow = nk_false;

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "nuklear_console_demo");
    SetWindowMinSize(200, 200);

    // Create the Nuklear Context
    int fontSize = 13 * 3;
    Font font = LoadFontFromNuklear(fontSize);
    GenTextureMipmaps(&font.texture);
    ctx = InitNuklearEx(font, fontSize);
    Texture texture = LoadTexture("resources/image.png");

    console = nuklear_console_demo_init(ctx, NULL, TextureToNuklear(texture));

    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
    #else
        while (!WindowShouldClose()) {
            UpdateDrawFrame();

            if (closeWindow) {
                break;
            }
        }
    #endif

    // De-initialize the Nuklear GUI
    UnloadTexture(texture);
    nuklear_console_demo_free();
    UnloadNuklear(ctx);
    UnloadFont(font);

    CloseWindow();
    return 0;
}

void UpdateDrawFrame(void) {
    // Update the Nuklear context, along with input
    UpdateNuklear(ctx);

    nk_gamepad_update(nk_console_get_gamepads(console));

    int flags = NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_TITLE;
    int padding = 0;

    // Nuklear GUI Code
    if (nk_begin(ctx, "nuklear_console", nk_rect(padding, padding, GetScreenWidth() - padding * 2, GetScreenHeight() - padding * 2), flags)) {
        if (nuklear_console_demo_render()) {
            closeWindow = nk_true;
        }
    }
    nk_end(ctx);

    // Render
    BeginDrawing();
        ClearBackground(BLACK);

        // Render the Nuklear GUI
        DrawNuklear(ctx);

    EndDrawing();

    #ifdef PLATFORM_WEB
        if (shouldClose) {
            emscripten_cancel_main_loop();
        }
    #endif
}
