#include "../../vendor/Nuklear/demo/common/style.c"

#define NK_GAMEPAD_IMPLEMENTATION
#include "../../vendor/nuklear_gamepad/nuklear_gamepad.h"

#define NK_CONSOLE_IMPLEMENTATION
#include "../../nuklear_console.h"

static struct nk_console* console = NULL;
static nk_size progressValue = 50;
static int weapon = 1;
static int property_int_test = 20;
static float property_float_test = 0.4f;
static int slider_int_test = 20;
static float slider_float_test = 0.4f;
static int theme = 5;
static nk_bool checkbox1 = nk_false;
static nk_bool checkbox2 = nk_false;
static nk_bool checkbox3 = nk_false;
static nk_bool shouldClose = nk_false;

static const int textedit_buffer_size = 256;
static char textedit_buffer[256] = "vurtun";

void button_clicked(struct nk_console* button) {
    if (strcmp(nk_console_get_label(button), "Quit Game") == 0) {
        shouldClose = nk_true;
    }
}

void theme_changed(struct nk_console* combobox) {
    set_style(combobox->ctx, theme);
}

nk_console* nuklear_console_demo_init(struct nk_context* ctx, void* user_data, struct nk_image image) {
    console = nk_console_init(ctx);
    nk_console_set_gamepad(console, nk_gamepad_init(ctx, user_data));

    // New Game
    nk_console* newgame = nk_console_button(console, "New Game");
    {
        nk_console_button_set_symbol(newgame, NK_SYMBOL_PLUS);
        nk_console_label(newgame, "This would start a new game!");
        nk_console_button_onclick(newgame, "Back", nk_console_button_back);
    }

    // Options
    nk_console* options = nk_console_button(console, "Options");
    {
        nk_console_set_tooltip(options, "Displays some random options!");

        nk_console* label_button = nk_console_button(options, "Labels");
        {
            nk_console_label(label_button, "This is a label that should wrap across multiple lines.")
                ->height = 128;
            nk_console_label(label_button, "This is a disabled label")
                ->disabled = nk_true;
            nk_console_label(label_button, "Center Aligned Label!")
                ->alignment = NK_TEXT_CENTERED;
            nk_console_label(label_button, "Right Aligned Label!")
                ->alignment = NK_TEXT_RIGHT;
            nk_console_button_onclick(label_button, "Back", nk_console_button_back);
        }

        nk_console* checkbox_button = nk_console_button(options, "Checkboxes");
        {
            nk_console_checkbox(checkbox_button, "Checkbox", &checkbox1)
                ->tooltip = "This is a checkbox!";
            nk_console_checkbox(checkbox_button, "Right aligned", &checkbox3)
                ->alignment = NK_TEXT_RIGHT;
            nk_console_checkbox(checkbox_button, "Disabled Checkbox", &checkbox2)
                ->disabled = nk_true;
            nk_console_button_onclick(checkbox_button, "Back", nk_console_button_back);
        }

        nk_console_progress(options, "Progress", &progressValue, 100);
        nk_console_combobox(options, "ComboBox", "Fists;Chainsaw;Pistol;Shotgun;Chaingun", ';', &weapon)
            ->tooltip = "Choose a weapon! The chainsaw is the best!";
        nk_console_property_int(options, "Property Int", 10, &property_int_test, 30, 1, 1);
        nk_console_property_float(options, "Property Float", 0.0f, &property_float_test, 2.0f, 0.1f, 1);
        nk_console_slider_float(options, "Slider Float", 0.0f, &slider_float_test, 2.0f, 0.1f)->tooltip = "Slider float is cool! It's what you want to use.";
        nk_console_slider_int(options, "Slider Int", 0, &slider_int_test, 20, 1)->disabled = nk_true;

        nk_console* textedit = nk_console_textedit(options, "Username", textedit_buffer, textedit_buffer_size);
        nk_console_set_tooltip(textedit, "Enter your username!");

        nk_console* image_button = nk_console_button(options, "Image");
        nk_console_button_set_image(image_button, image);
        image_button->height = 128;

        nk_console_button_set_symbol(
            nk_console_button_onclick(options, "Back", nk_console_button_back),
            NK_SYMBOL_TRIANGLE_LEFT);
    }

    nk_console* theme_options = nk_console_combobox(console, "Theme", "Black;White;Red;Blue;Dark;Dracula;Default", ';', &theme);
    theme_options->onchange = theme_changed;
    theme_options->tooltip = "Change the theme of the console!";
    set_style(ctx, theme);

    // Rows
    nk_console* calc = nk_console_button(console, "Calculator");
    {
      nk_console* row = nk_console_row_begin(calc);
      nk_console_button(row, "sqrt");
      nk_console_button(row, "pi");
      nk_console_row_end(row);

      row = nk_console_row_begin(calc);
      nk_console_button(row, "AC");
      nk_console_button(row, "()");
      nk_console_button(row, "%");
      nk_console_button(row, "/");
      nk_console_row_end(row);

      row = nk_console_row_begin(calc);
      nk_console_button(row, "7");
      nk_console_button(row, "8");
      nk_console_button(row, "9");
      nk_console_button(row, "*");
      nk_console_row_end(row);

      row = nk_console_row_begin(calc);
      nk_console_button(row, "4");
      nk_console_button(row, "5");
      nk_console_button(row, "6");
      nk_console_button(row, "-");
      nk_console_row_end(row);

      row = nk_console_row_begin(calc);
      nk_console_button(row, "1");
      nk_console_button(row, "2");
      nk_console_button(row, "3");
      nk_console_button(row, "+");
      nk_console_row_end(row);

      row = nk_console_row_begin(calc);
      nk_console_button(row, "0");
      nk_console_button(row, ".");
      nk_console_button(row, "bksp");
      nk_console_button(row, "=");
      nk_console_row_end(row);

      nk_console_button_set_symbol(
        nk_console_button_onclick(calc, "Back", nk_console_button_back),
        NK_SYMBOL_TRIANGLE_LEFT);

      calc->tooltip = "Demo rows and grids!";
    }

    nk_console_button(console, "Save Game")->disabled = nk_true;
    nk_console_button_onclick(console, "Quit Game", button_clicked);

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
