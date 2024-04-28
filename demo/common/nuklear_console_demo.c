#include "../../nuklear_console.h"
#include "../../vendor/Nuklear/demo/common/style.c"

struct nk_console* console;
static nk_size progressValue = 50;
static int weapon = 1;
static int property_int_test = 20;
static float property_float_test = 0.4f;
static int slider_int_test = 20;
static float slider_float_test = 0.4f;
static int theme = 4;
nk_bool showWindowTitle = nk_true;
nk_bool shouldClose = nk_false;

void button_clicked(struct nk_console* button) {
    if (strcmp(button->text, "Quit Game") == 0) {
        shouldClose = nk_true;
    }
}

void theme_changed(struct nk_console* combobox) {
    set_style(combobox->context, theme);
}

nk_console* nuklear_console_demo_init(struct nk_context* ctx, void* user_data) {
    console = nk_console_init(ctx);
    nk_console_set_gamepad(console, nk_gamepad_init(ctx, user_data));

    // New Game
    nk_console* newgame = nk_console_button(console, "New Game");
    {
        newgame->button.symbol = NK_SYMBOL_PLUS;
        nk_console_label(newgame, "This would start a new game!");
        nk_console_button_onclick(newgame, "Back", nk_console_button_back);
    }

    // Options
    nk_console* options = nk_console_button(console, "Options");
    {
        nk_console_checkbox(options, "Show Window Title", &showWindowTitle)
            ->tooltip = "Whether or not to show the window title";
        nk_console_progress(options, "Progress", &progressValue, 100);
        nk_console_combobox(options, "ComboBox", "Fists;Chainsaw;Pistol;Shotgun;Chaingun", ';', &weapon)
            ->tooltip = "Choose a weapon! The chainsaw is the best!";
        nk_console_property_int(options, "Property Int", 10, &property_int_test, 30, 1, 1);
        nk_console_property_float(options, "Property Float", 0.0f, &property_float_test, 2.0f, 0.1f, 1);
        nk_console_slider_float(options, "Slider Float", 0.0f, &slider_float_test, 2.0f, 0.1f)->tooltip = "Slider float is cool! It's what you want to use.";
        nk_console_slider_int(options, "Slider Int", 0, &slider_int_test, 20, 1)->disabled = nk_true;

        nk_console_button_onclick(options, "Back", nk_console_button_back)
            ->button.symbol = NK_SYMBOL_TRIANGLE_LEFT;

        options->tooltip = "Displays some random options!";
    }

    nk_console* theme_options = nk_console_combobox(console, "Theme", "Black;White;Red;Blue;Dark;Dracula;Default", ';', &theme);
    theme_options->onchange = theme_changed;
    theme_options->tooltip = "Change the theme of the console!";
    set_style(ctx, theme);

    nk_console_button(console, "Save Game")->disabled = nk_true;
    nk_console_button(console, "Quit Game")->button.onclick = button_clicked;

    return console;
}

nk_bool nuklear_console_demo_render() {
    nk_console_render(console);

    return shouldClose;;
}

void nuklear_console_demo_free() {
    nk_gamepad_free(console->gamepads);
    nk_console_free(console);
}
