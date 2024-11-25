#include <stdio.h> // snprintf
#include <string.h> // strcmp

#include "../../vendor/Nuklear/demo/common/style.c"

#define NK_GAMEPAD_IMPLEMENTATION
#include "../../vendor/nuklear_gamepad/nuklear_gamepad.h"

#define NK_CONSOLE_IMPLEMENTATION
#include "../../nuklear_console.h"

// Demo
static struct nk_context* ctx;
static struct nk_console* console;
static struct nk_gamepads gamepads;
static nk_bool shouldClose = nk_false;

struct demo_console_state {
    char title[64];

    // Theme
    int theme;

    // Progress
    nk_size progressValue;

    // Combobox
    int weapon;

    // Property
    int property_int_test;
    float property_float_test;

    // Slider
    int slider_int_test;
    float slider_float_test;

    // Radio
    int radio_option;
    int radio_option2;

    // Checkbox
    nk_bool checkbox1;
    nk_bool checkbox2;
    nk_bool checkbox3;
    nk_bool checkbox4;
    nk_bool checkbox5;
    nk_bool checkbox6;

    // Messages
    int message_count;

    // File
    char file_path_buffer[1024];
    int file_path_buffer_size;

    // Textedit
    int textedit_buffer_size;
    char textedit_buffer[256];

    // Input
    int gamepad_number;
    enum nk_gamepad_button gamepad_button;

    // Color
    struct nk_colorf color;
};

static struct demo_console_state demo_console_state_defaults(void) {
    struct demo_console_state state = {
        .theme = THEME_DRACULA,
        .progressValue = 50,
        .weapon = 1,
        .property_int_test = 20,
        .property_float_test = 0.4f,
        .slider_int_test = 20,
        .slider_float_test = 0.4f,
        .radio_option = 1,
        .radio_option2 = 0,
        .checkbox1 = nk_false,
        .checkbox2 = nk_false,
        .checkbox3 = nk_false,
        .checkbox4 = nk_false,
        .checkbox5 = nk_false,
        .checkbox6 = nk_true,
        .message_count = 0,
        .file_path_buffer_size = 1024,
        .textedit_buffer_size = 256,
        .color = {0.31f, 1.0f, 0.48f, 1.0f},
        .gamepad_number = 0,
        .gamepad_button = NK_GAMEPAD_BUTTON_A,
    };

    return state;
}

void button_clicked(struct nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    if (strcmp(nk_console_get_label(button), "Quit Game") == 0) {
        shouldClose = nk_true;
    }
}

void theme_changed(struct nk_console* combobox, void* user_data) {
    enum theme t = *(enum theme*)user_data;
    set_style(combobox->ctx, t);
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
    struct demo_console_state* state = (struct demo_console_state*)user_data;
    char message[128];
    snprintf(message, 128, "This is message #%d!", ++state->message_count);
    nk_console_show_message(button, message);
}

/**
 * Triggered when clicking the "Multiple Windows" button.
 */
void multiple_window_button_clicked(nk_console* button, void* userdata) {
    nk_console* top = userdata;
    for (int i = 0; i < cvector_size(top->children); i++) {
        if (top->children[i]->type == NK_CONSOLE_WINDOW) {
            nk_console_window_data* data = top->children[i]->data;

            // Display the child windows.
            data->flags = 0;
        }
    }
}

void nk_console_radio_changed(struct nk_console* radio, void* user_data) {
    NK_UNUSED(user_data);
    nk_console_show_message(radio, radio->label);
}

void nuklear_console_demo_init(struct nk_console* _console, struct demo_console_state* state, void* user_data, struct nk_image image) {
    nk_console* top = nk_console_get_top(_console);

    nk_gamepad_init(&gamepads, ctx, user_data);
    nk_console_set_gamepads(top, &gamepads);

    // New Game
    nk_console* newgame = nk_console_button(_console, "New Game");
    {
        nk_console_button_set_symbol(newgame, NK_SYMBOL_PLUS);
        nk_console_label(newgame, "This would start a new game!");
        nk_console_button_onclick(newgame, "Back", &nk_console_button_back);
    }

    // Widgets
    nk_console* widgets = nk_console_button(_console, "Widgets");
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
            nk_console_checkbox(checkbox_button, "Checkbox", &state->checkbox1)
                ->tooltip = "This is a checkbox!";
            nk_console_checkbox(checkbox_button, "Right aligned", &state->checkbox3)
                ->alignment = NK_TEXT_RIGHT;
            nk_console_checkbox(checkbox_button, "Disabled Checkbox", &state->checkbox2)
                ->disabled = nk_true;

            // Onchange callbacks can be used to implement custom logic.
            // These two checkboxes disable each other when checked.
            nk_console* exclude_a = nk_console_checkbox(checkbox_button, "Exclusive A (disables B)", &state->checkbox4);
            nk_console* exclude_b = nk_console_checkbox(checkbox_button, "Exclusive B (disables A)", &state->checkbox5);
            nk_console_add_event_handler(exclude_a, NK_CONSOLE_EVENT_CHANGED, &exclude_other_checkbox, exclude_b, NULL);
            nk_console_add_event_handler(exclude_b, NK_CONSOLE_EVENT_CHANGED, &exclude_other_checkbox, exclude_a, NULL);

            // Checkbox that will show/hide the below label.
            nk_console* checkbox_show_label = nk_console_checkbox(checkbox_button, "Show Label", &state->checkbox6);
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
            nk_console_radio(radios, "Radio #1", &state->radio_option);
            nk_console_radio(radios, "Radio #2", &state->radio_option);
            nk_console_radio(radios, "Radio #3", &state->radio_option);
            nk_console_radio(radios, "Radio #4", &state->radio_option);

            nk_console_label(radios, "Option B:");
            nk_console_radio(radios, "Left Aligned #1", &state->radio_option2)->alignment = NK_TEXT_LEFT;
            nk_console_radio(radios, "Left Aligned #2", &state->radio_option2)->alignment = NK_TEXT_LEFT;
            nk_console_radio(radios, "Center Aligned #1", &state->radio_option2)->alignment = NK_TEXT_CENTERED;
            nk_console_radio(radios, "Center Aligned #2", &state->radio_option2)->alignment = NK_TEXT_CENTERED;
            nk_console_radio(radios, "Right Aligned #1", &state->radio_option2)->alignment = NK_TEXT_RIGHT;
            nk_console_radio(radios, "Right Aligned #2", &state->radio_option2)->alignment = NK_TEXT_RIGHT;
            nk_console_radio(radios, "Disabled", &state->radio_option2)->disabled = nk_true;
            nk_console_radio(radios, "Invisible", &state->radio_option2)->visible = nk_false;

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
            nk_console* b = nk_console_button(row, "");
            nk_console_button_set_symbol(b, NK_SYMBOL_TRIANGLE_UP);
            nk_console_spacing(row, 1);
            nk_console_row_end(row);

            row = nk_console_row_begin(spacing);
            b = nk_console_button(row, "");
            nk_console_button_set_symbol(b, NK_SYMBOL_TRIANGLE_LEFT);
            nk_console_spacing(row, 1);
            b = nk_console_button(row, "");
            nk_console_button_set_symbol(b, NK_SYMBOL_TRIANGLE_RIGHT);
            nk_console_row_end(row);

            row = nk_console_row_begin(spacing);
            nk_console_spacing(row, 1);
            b = nk_console_button(row, "");
            nk_console_button_set_symbol(b, NK_SYMBOL_TRIANGLE_DOWN);
            nk_console_spacing(row, 1);
            nk_console_row_end(row);

            nk_console_button_onclick(spacing, "Back", &nk_console_button_back);
        }

        // Progress Bar
        nk_console* progressbar = nk_console_button(widgets, "Progress Bar");
        {
            nk_console_progress(progressbar, "Progress", &state->progressValue, 100);
            nk_console_button_onclick(progressbar, "Back", &nk_console_button_back);
        }

        // Input: From any gamepad (-1)
        nk_console_input(widgets, "Input Button", -1, &state->gamepad_number, &state->gamepad_button);

        // Combobox
        nk_console_combobox(widgets, "ComboBox", "Fists;Chainsaw;Pistol;Shotgun;Chaingun", ';', &state->weapon)
            ->tooltip = "Choose a weapon! The chainsaw is the best!";

        // Property
        nk_console* properties = nk_console_button(widgets, "Property");
        {
            nk_console_property_int(properties, "Property Int", 10, &state->property_int_test, 30, 1, 1);
            nk_console_property_float(properties, "Property Float", 0.0f, &state->property_float_test, 2.0f, 0.1f, 1);
            nk_console_button_onclick(properties, "Back", &nk_console_button_back);
        }

        // Sliders
        nk_console* sliders = nk_console_button(widgets, "Sliders");
        {
            nk_console_slider_float(sliders, "Slider Float", 0.0f, &state->slider_float_test, 2.0f, 0.1f)->tooltip = "Slider float is cool! It's what you want to use.";
            nk_console_slider_int(sliders, "Slider Int", 0, &state->slider_int_test, 20, 1);
            nk_console_button_onclick(sliders, "Back", &nk_console_button_back);
        }

        // Textedit
        nk_console* textedit = nk_console_textedit(widgets, "Username", state->textedit_buffer, state->textedit_buffer_size);
        nk_console_set_tooltip(textedit, "Enter your username!");

        // Color
        nk_console_color(widgets, "Select Color", &state->color, NK_RGBA);

        // File
        nk_console_file(widgets, "File", state->file_path_buffer, state->file_path_buffer_size);

        // Messages
        nk_console_button_onclick_handler(widgets, "Show Message", &nk_console_demo_show_message, &state, NULL);

        // Back Button
        nk_console_button_set_symbol(
            nk_console_button_onclick(widgets, "Back", &nk_console_button_back),
            NK_SYMBOL_TRIANGLE_LEFT);
    }

    nk_console* theme_options = nk_console_combobox(_console, "Theme", "Black;White;Red;Blue;Dark;Dracula;Default", ';', &state->theme);
    nk_console_add_event_handler(theme_options, NK_CONSOLE_EVENT_CHANGED, &theme_changed, &state->theme, NULL);
    theme_options->tooltip = "Change the theme of the console!";
    set_style(ctx, (enum theme)state->theme);

    // Rows
    nk_console* calc = nk_console_button(_console, "Calculator");
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

    // Multiple Windows
    {
        nk_console* window1 = nk_console_window(_console, "Window #1", nk_rect(10, 10, 100, 100), NK_WINDOW_HIDDEN);
        nk_console* window2 = nk_console_window(_console, "Window #2", nk_rect(110, 10, 100, 100), NK_WINDOW_HIDDEN);
        nk_console* multiple_window_button = nk_console_button_onclick_handler(_console, "Multiple Windows", &multiple_window_button_clicked, _console, NULL);
        //nk_console_button_onclick(_console, "Multiple Windows", &multiple_window_button_clicked);
    }

    nk_console_button(_console, "Save Game")->disabled = nk_true;
    nk_console_button_onclick(_console, "Quit Game", &button_clicked);
}

nk_bool nuklear_console_demo_render() {
    nk_console_render(console);

    return shouldClose;
}

void nuklear_console_demo_free() {
    nk_gamepad_free(nk_console_get_gamepads(console));
    nk_console_free(console);
}
