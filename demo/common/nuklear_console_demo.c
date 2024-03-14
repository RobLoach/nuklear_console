#include <string.h>

#include "../../nuklear_console.h"

struct nk_console* console;
static nk_size progressValue = 50;

void button_clicked(struct nk_console* button) {
    if (strcmp(button->text, "Quit Game") == 0) {
        // Leverage the userdata to indicate whether or not we're to quit.
        button->context->userdata.id = 1;
    }
}

void nuklear_console_demo_init(struct nk_context* ctx) {
    console = nk_console_init(ctx);

    // New Game
    nk_console* newgame = nk_console_add_button(console, "New Game");
    {
        nk_console_add_label(newgame, "This would start a new game!");
        nk_console_add_button_onclick(newgame, "Back", nk_console_onclick_back);
    }

    // Options
    nk_console* options = nk_console_add_button(console, "Options");
    {
        nk_console_add_checkbox(options, "Show Window Title", &showWindowTitle);
        nk_console_add_progress(options, "Number", &progressValue, 100);
        nk_console_add_button_onclick(options, "Back", nk_console_onclick_back);
    }

    nk_console_add_button(console, "Load Game");
    nk_console_add_button(console, "Save Game");
    nk_console_add_button(console, "Quit Game")->onclick = button_clicked;
}

nk_bool nuklear_console_demo_render() {
    nk_console_render(console);

    if (console->context->userdata.id == 1) {
        return nk_true;
    }

    return nk_false;
}

void nuklear_console_demo_free() {
    nk_console_free(console);
}
