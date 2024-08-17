#include <string.h> // strcmp
#include <stdio.h> // snprintf

#include "../../vendor/Nuklear/demo/common/style.c"

#define NK_GAMEPAD_IMPLEMENTATION
#include "../../vendor/nuklear_gamepad/nuklear_gamepad.h"

#define NK_CONSOLE_IMPLEMENTATION
#include "../../nuklear_console.h"

// Demo
static struct nk_console* console = NULL;
static struct nk_gamepads gamepads;
static nk_bool shouldClose = nk_false;

// Theme
static int theme = THEME_DRACULA;

// Progress
static nk_size progressValue = 50;

// Combobox
static int weapon = 1;

// Property
static int property_int_test = 20;
static float property_float_test = 0.4f;

// Slider
static int slider_int_test = 20;
static float slider_float_test = 0.4f;

// Radio
static int radio_option = 1;
static int radio_option2 = 0;

// Checkbox
static nk_bool checkbox1 = nk_false;
static nk_bool checkbox2 = nk_false;
static nk_bool checkbox3 = nk_false;
static nk_bool checkbox4 = nk_false;
static nk_bool checkbox5 = nk_false;
static nk_bool checkbox6 = nk_true;

// Messages
static int message_count = 0;

// File
static char file_path_buffer[1024] = {0};
static int file_path_buffer_size = 1024;

// Textedit
static const int textedit_buffer_size = 256;
static char textedit_buffer[256] = "vurtun";

// Input
static int gamepad_number = 0;
static enum nk_gamepad_button gamepad_button = NK_GAMEPAD_BUTTON_A;

// Color
static struct nk_colorf color = {0.31f, 1.0f, 0.48f, 1.0f};

void button_clicked(struct nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    if (strcmp(nk_console_get_label(button), "Quit Game") == 0) {
        shouldClose = nk_true;
    }
}

void theme_changed(struct nk_console* combobox, void* user_data) {
    NK_UNUSED(user_data);
    set_style(combobox->ctx, (enum theme)theme);
}

void exclude_other_checkbox(nk_console* unused, void* user_data) {
    NK_UNUSED(unused);
    nk_console* other = (nk_console*)user_data;
    other->disabled = !other->disabled;
}

void toggle_visibility(nk_console* unused, void* user_data) {
    NK_UNUSED(unused);
    nk_console* other = (nk_console*)user_data;
    other->visible = !other->visible;
}

void nk_console_demo_show_message(struct nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    char message[128];
    snprintf(message, 128, "This is message #%d!", ++message_count);
    nk_console_show_message(button, message);
}

void nk_console_radio_changed(struct nk_console* radio, void* user_data) {
    NK_UNUSED(user_data);
    nk_console_show_message(radio, radio->label);
}

nk_console* nuklear_console_demo_init(struct nk_context* ctx, void* user_data, struct nk_image image) {
    console = nk_console_init(ctx);

    nk_gamepad_init(&gamepads, ctx, user_data);
    nk_console_set_gamepads(console, &gamepads);

    // New Game
    nk_console* newgame = nk_console_button(console, "New Game");
    {
        nk_console_button_set_symbol(newgame, NK_SYMBOL_PLUS);
        nk_console_label(newgame, "This would start a new game!");
        nk_console_button_onclick(newgame, "Back", &nk_console_button_back);
    }

    // Widgets
    nk_console* widgets = nk_console_button(console, "Widgets");
    {
        nk_console_set_tooltip(widgets, "Displays some random options!");

        nk_console* label_button = nk_console_button(widgets, "Labels");
        {
            nk_console_label(label_button, "Simple label.");
            nk_console* label1 = nk_console_label(label_button, "Selectable label #1");
            label1->selectable = nk_true;
            nk_console_add_event(label1, NK_CONSOLE_EVENT_CLICKED, &nk_console_demo_show_message);

            nk_console_label(label_button, "Selectable label #2.")
                ->selectable = nk_true;
            nk_console_label(label_button, "This is a label that will wrap across multiple lines.")
                ->height = 128;
            nk_console_label(label_button, "This is a disabled label")
                ->disabled = nk_true;
            nk_console_label(label_button, "Center Aligned Label!")
                ->alignment = NK_TEXT_CENTERED;
            nk_console_label(label_button, "Right Aligned Label!")
                ->alignment = NK_TEXT_RIGHT;
            nk_console_button_onclick(label_button, "Back", &nk_console_button_back);
        }

        nk_console* checkbox_button = nk_console_button(widgets, "Checkboxes");
        {
            nk_console_checkbox(checkbox_button, "Checkbox", &checkbox1)
                ->tooltip = "This is a checkbox!";
            nk_console_checkbox(checkbox_button, "Right aligned", &checkbox3)
                ->alignment = NK_TEXT_RIGHT;
            nk_console_checkbox(checkbox_button, "Disabled Checkbox", &checkbox2)
                ->disabled = nk_true;

            // Onchange callbacks can be used to implement custom logic.
            // These two checkboxes disable each other when checked.
            nk_console* exclude_a = nk_console_checkbox(checkbox_button, "Exclusive A (disables B)", &checkbox4);
            nk_console* exclude_b = nk_console_checkbox(checkbox_button, "Exclusive B (disables A)", &checkbox5);
            nk_console_add_event_handler(exclude_a, NK_CONSOLE_EVENT_CHANGED, &exclude_other_checkbox, exclude_b, NULL);
            nk_console_add_event_handler(exclude_b, NK_CONSOLE_EVENT_CHANGED, &exclude_other_checkbox, exclude_a, NULL);

            // Checkbox that will show/hide the below label.
            nk_console* checkbox_show_label = nk_console_checkbox(checkbox_button, "Show Label", &checkbox6);
            nk_console* label_to_show = nk_console_label(checkbox_button, "This label is only shown when the checkbox is checked.");
            nk_console_add_event_handler(checkbox_show_label, NK_CONSOLE_EVENT_CHANGED, &toggle_visibility, label_to_show, NULL);

            nk_console_button_onclick(checkbox_button, "Back", &nk_console_button_back);
        }

        nk_console* buttons = nk_console_button(widgets, "Buttons");
        {
            nk_console_button(buttons, "Button");
            nk_console_button(buttons, "Button #2");
            nk_console_button(buttons, "Disabled Button")
                ->disabled = nk_true;

            // Image Button
            nk_console* image_button = nk_console_button(buttons, "Image");
            nk_console_button_set_image(image_button, image);
            image_button->height = 128;

            nk_console_button_onclick(buttons, "Back", &nk_console_button_back);
        }

        // Radio Buttons
        nk_console* radios = nk_console_button(widgets, "Radios");
        {
            nk_console_label(radios, "Option A:");
            nk_console_radio(radios, "Radio #1", &radio_option);
            nk_console_radio(radios, "Radio #2", &radio_option);
            nk_console_radio(radios, "Radio #3", &radio_option);
            nk_console_radio(radios, "Radio #4", &radio_option);

            nk_console_label(radios, "Option B:");
            nk_console_radio(radios, "Left Aligned #1", &radio_option2)->alignment = NK_TEXT_LEFT;
            nk_console_radio(radios, "Left Aligned #2", &radio_option2)->alignment = NK_TEXT_LEFT;
            nk_console_radio(radios, "Center Aligned #1", &radio_option2)->alignment = NK_TEXT_CENTERED;
            nk_console_radio(radios, "Center Aligned #2", &radio_option2)->alignment = NK_TEXT_CENTERED;
            nk_console_radio(radios, "Right Aligned #1", &radio_option2)->alignment = NK_TEXT_RIGHT;
            nk_console_radio(radios, "Right Aligned #2", &radio_option2)->alignment = NK_TEXT_RIGHT;
            nk_console_radio(radios, "Disabled", &radio_option2)->disabled = nk_true;
            nk_console_radio(radios, "Invisible", &radio_option2)->visible = nk_false;

            nk_console_button_onclick(radios, "Back", &nk_console_button_back);
        }

        // Images
        nk_console* images = nk_console_button(widgets, "Images");
        {
            nk_console* img = nk_console_image(images, image);
            nk_console_set_height(img, image.h);
            img = nk_console_image_color(images, image, nk_rgb(255, 0, 0));
            nk_console_set_height(img, image.h);

            nk_console_button_onclick(images, "Back", &nk_console_button_back);
        }

        // Spacing
        nk_console* spacing = nk_console_button(widgets, "Spacing");
        {
            nk_console* row = nk_console_row_begin(spacing);
            nk_console_spacing(row, 1);
            nk_console* b = nk_console_button(row,"");
            nk_console_button_set_symbol(b, NK_SYMBOL_TRIANGLE_UP);
            nk_console_spacing(row, 1);
            nk_console_row_end(row);

            row = nk_console_row_begin(spacing);
            b = nk_console_button(row,"");
            nk_console_button_set_symbol(b, NK_SYMBOL_TRIANGLE_LEFT);
            nk_console_spacing(row, 1);
            b = nk_console_button(row,"");
            nk_console_button_set_symbol(b, NK_SYMBOL_TRIANGLE_RIGHT);
            nk_console_row_end(row);

            row = nk_console_row_begin(spacing);
            nk_console_spacing(row, 1);
            b = nk_console_button(row,"");
            nk_console_button_set_symbol(b, NK_SYMBOL_TRIANGLE_DOWN);
            nk_console_spacing(row, 1);
            nk_console_row_end(row);

            nk_console_button_onclick(spacing, "Back", &nk_console_button_back);
        }

        // Progress Bar
        nk_console* progressbar = nk_console_button(widgets, "Progress Bar");
        {
            nk_console_progress(progressbar, "Progress", &progressValue, 100);
            nk_console_button_onclick(progressbar, "Back", &nk_console_button_back);
        }

        // Input: From any gamepad (-1)
        nk_console_input(widgets, "Input Button", -1, &gamepad_number, &gamepad_button);

        // Combobox
        nk_console_combobox(widgets, "ComboBox", "Fists;Chainsaw;Pistol;Shotgun;Chaingun", ';', &weapon)
            ->tooltip = "Choose a weapon! The chainsaw is the best!";

        // Property
        nk_console* properties = nk_console_button(widgets, "Property");
        {
            nk_console_property_int(properties, "Property Int", 10, &property_int_test, 30, 1, 1);
            nk_console_property_float(properties, "Property Float", 0.0f, &property_float_test, 2.0f, 0.1f, 1);
            nk_console_button_onclick(properties, "Back", &nk_console_button_back);
        }

        // Sliders
        nk_console* sliders = nk_console_button(widgets, "Sliders");
        {
            nk_console_slider_float(sliders, "Slider Float", 0.0f, &slider_float_test, 2.0f, 0.1f)->tooltip = "Slider float is cool! It's what you want to use.";
            nk_console_slider_int(sliders, "Slider Int", 0, &slider_int_test, 20, 1);
            nk_console_button_onclick(sliders, "Back", &nk_console_button_back);
        }

        // Textedit
        nk_console* textedit = nk_console_textedit(widgets, "Username", textedit_buffer, textedit_buffer_size);
        nk_console_set_tooltip(textedit, "Enter your username!");

        // Color
        nk_console_color(widgets, "Select Color", &color, NK_RGBA);

        // File
        nk_console_file(widgets, "File", file_path_buffer, file_path_buffer_size);

        // Messages
        nk_console_button_onclick(widgets, "Show Message", &nk_console_demo_show_message);

        // Back Button
        nk_console_button_set_symbol(
            nk_console_button_onclick(widgets, "Back", &nk_console_button_back),
            NK_SYMBOL_TRIANGLE_LEFT);
    }

    nk_console* theme_options = nk_console_combobox(console, "Theme", "Black;White;Red;Blue;Dark;Dracula;Default", ';', &theme);
    nk_console_add_event(theme_options, NK_CONSOLE_EVENT_CHANGED, &theme_changed);
    theme_options->tooltip = "Change the theme of the console!";
    set_style(ctx, (enum theme)theme);

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
        nk_console_button_onclick(calc, "Back", &nk_console_button_back),
        NK_SYMBOL_TRIANGLE_LEFT);

      calc->tooltip = "Demo rows and grids!";
    }

    nk_console_button(console, "Save Game")->disabled = nk_true;
    nk_console_button_onclick(console, "Quit Game", &button_clicked);

    return console;
}

nk_bool nuklear_console_demo_render() {
    nk_console_render(console);

    return shouldClose;;
}

void nuklear_console_demo_free() {
    nk_gamepad_free(nk_console_get_gamepads(console));
    nk_console_free(console);
}
