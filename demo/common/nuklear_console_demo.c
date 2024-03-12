#include <string.h>

#define NK_CONSOLE_IMPLEMENTATION
#include "../../nuklear_console.h"

struct nk_console* console;

void button_clicked(struct nk_console* button) {
    if (strcmp(button->text, "Quit Game") == 0) {
        exit(0);
    }
}

void nuklear_console_demo_init(struct nk_context* ctx) {
    console = nk_console_init(ctx);
    nk_console_add_button(console, "New Game");
    nk_console* options = nk_console_add_button(console, "Options");
    {
        nk_console_add_button(options, "Some cool option!");
        nk_console_add_button(options, "Option #2");
        nk_console_add_button_onclick(options, "Back", nk_console_onclick_back);
    }
    nk_console_add_button(console, "Load Game");
    nk_console_add_button(console, "Save Game");
    nk_console_add_button(console, "Quit Game")->onclick = button_clicked;
}

void nuklear_console_demo_update() {
    nk_console_update(console);
}

void nuklear_console_demo_render() {
    nk_console_render(console);
}

void nuklear_console_demo_free() {
    nk_console_free(console);
}