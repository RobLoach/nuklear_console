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
struct nk_rect lastWindowSize;

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 450, "nuklear_console_demo");
    SetWindowMinSize(200, 200);

    // Create the Nuklear Context
    int fontSize = 13 * 2;
    Font font = LoadFontFromNuklear(fontSize);
    GenTextureMipmaps(&font.texture);
    ctx = InitNuklearEx(font, fontSize);
    Texture texture = LoadTexture("resources/image.png");

    console = nuklear_console_demo_init(ctx, NULL, TextureToNuklearImage(texture));
    lastWindowSize = nk_rect(0, 0, (float)GetScreenWidth() * 0.80f, (float)GetScreenHeight());

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

    int flags = NK_WINDOW_SCROLL_AUTO_HIDE;

    // Center the window using the previous frame's rendered size, capped to screen
    float windowW = lastWindowSize.w < GetScreenWidth() ? lastWindowSize.w : GetScreenWidth();
    float windowH = lastWindowSize.h < GetScreenHeight() ? lastWindowSize.h : GetScreenHeight();
    struct nk_rect centered = nk_rect(
        (GetScreenWidth() - windowW) / 2.0f,
        (GetScreenHeight() - windowH) / 2.0f,
        windowW,
        windowH
    );

    // Nuklear GUI Code
    lastWindowSize = nk_console_render_window(console, "nuklear_console", centered, flags);

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
    #else
        if (nuklear_console_demo_should_close()) {
            closeWindow = nk_true;
        }
    #endif
}
