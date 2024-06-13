#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define RAYLIB_NUKLEAR_INCLUDE_DEFAULT_FONT
#include "raylib-nuklear.h"

#include "../common/nuklear_console_demo.c"

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "nuklear_console_demo");
    SetTargetFPS(60);

    // Create the Nuklear Context
    int fontSize = 13 * 3;
    int padding = 25;
    Font font = LoadFontFromNuklear(fontSize);
    GenTextureMipmaps(&font.texture);
    struct nk_context *ctx = InitNuklearEx(font, fontSize);

    nk_console* console = nuklear_console_demo_init(ctx, NULL);

    while (!WindowShouldClose()) {

        // Update the Nuklear context, along with input
        UpdateNuklear(ctx);
        nk_gamepad_update(console->gamepads);

        int flags = NK_WINDOW_BORDER;
        flags |= NK_WINDOW_SCROLL_AUTO_HIDE;
        if (showWindowTitle) {
            flags |= NK_WINDOW_TITLE;
        }

        // Nuklear GUI Code
        if (nk_begin(ctx, "nuklear_console_demo", nk_rect(padding, padding, GetScreenWidth() - padding * 2, GetScreenHeight() - padding * 2), flags)) {
            if (nuklear_console_demo_render()) {
                break;
            }
        }
        nk_end(ctx);

        // Render
        BeginDrawing();
            ClearBackground(BLACK);

            // Render the Nuklear GUI
            DrawNuklear(ctx);

        EndDrawing();
    }

    // De-initialize the Nuklear GUI
    nuklear_console_demo_free();
    UnloadNuklear(ctx);
    UnloadFont(font);

    CloseWindow();
    return 0;
}
