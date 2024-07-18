#define RAYLIB_NUKLEAR_IMPLEMENTATION
#define RAYLIB_NUKLEAR_INCLUDE_DEFAULT_FONT
// TODO: Switch to Nuklear's allocator system.
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#include "raylib-nuklear.h"

#include "../../nuklear_console.h"

// File System (optional)
void nk_console_file_destroy_raylib(nk_console* console) {
    FilePathList* list = (FilePathList*)nk_console_file_get_file_user_data(console);
    if (list != NULL) {
        // Clear the directory entries.
        UnloadDirectoryFiles(*list);

        // Clear our the FilePathList data.
        MemFree(list);
    }
}
#define NK_CONSOLE_FILE_ADD_FILES nk_console_file_add_files_raylib

// Updates the list of files when selecting a file.
void nk_console_file_add_files_raylib(nk_console* console, const char* path) {
    FilePathList* list = nk_console_file_get_file_user_data(console);
    if (list == NULL) {
        list = (FilePathList*)MemAlloc(sizeof(FilePathList));
        nk_console_file_set_file_user_data(console, (void*)list);
        console->destroy = nk_console_file_destroy_raylib;
    }
    else {
        UnloadDirectoryFiles(*list);
    }

    TraceLog(LOG_INFO, "Loading files from directory: %s", path);
    FilePathList filePathList = LoadDirectoryFiles(path);
    for (int i = 0; i < filePathList.count; i++) {
        TraceLog(LOG_INFO, "  %s", filePathList.paths[i]);
        nk_console_file_add_entry(console, filePathList.paths[i], DirectoryExists(filePathList.paths[i]));
    }
    *list = filePathList;
}

#include "../common/nuklear_console_demo.c"

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "nuklear_console_demo");
    SetWindowMinSize(200, 200);

    // Create the Nuklear Context
    int fontSize = 13 * 3;
    int padding = 0;
    Font font = LoadFontFromNuklear(fontSize);
    GenTextureMipmaps(&font.texture);
    struct nk_context *ctx = InitNuklearEx(font, fontSize);
    Texture texture = LoadTexture("resources/image.png");

    nk_console* console = nuklear_console_demo_init(ctx, NULL, TextureToNuklear(texture));

    while (!WindowShouldClose()) {

        // Update the Nuklear context, along with input
        UpdateNuklear(ctx);

        nk_gamepad_update(nk_console_get_gamepads(console));

        int flags = NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_TITLE;

        // Nuklear GUI Code
        if (nk_begin(ctx, "nuklear_console", nk_rect(padding, padding, GetScreenWidth() - padding * 2, GetScreenHeight() - padding * 2), flags)) {
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
    UnloadTexture(texture);
    nuklear_console_demo_free();
    UnloadNuklear(ctx);
    UnloadFont(font);

    CloseWindow();
    return 0;
}
