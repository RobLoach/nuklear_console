// This test suite uses pntr_nuklear to render the Nuklear Context to an image.
// https://github.com/RobLoach/pntr_nuklear

#include <assert.h>
#include <stdio.h>

#define PNTR_ENABLE_DEFAULT_FONT
#define PNTR_IMPLEMENTATION
#include "pntr.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define PNTR_NUKLEAR_IMPLEMENTATION
#include "pntr_nuklear.h"

#define NK_GAMEPAD_NONE
#define NK_GAMEPAD_IMPLEMENTATION
#include "nuklear_gamepad.h"

#define NK_CONSOLE_IMPLEMENTATION
#include "nuklear_console.h"

int main() {
    // Load Nuklear
    pntr_font* font = pntr_load_font_default();
    assert(font != NULL);
    struct nk_context* ctx = pntr_load_nuklear(font);
    assert(ctx != NULL);

    // nk_console_init()
    struct nk_console* console = nk_console_init(ctx);
    assert(console != NULL);

    // Gamepad
    struct nk_gamepads gamepads;
    assert(nk_gamepad_init(&gamepads, ctx, NULL) == nk_true);
    nk_console_set_gamepads(console, &gamepads);

    // nk_console_label()
    nk_console* label = nk_console_label(console, "Simple label.");
    assert(label != NULL);

    // nk_console_button()
    nk_console* button = nk_console_button(console, "Button");
    assert(button != NULL);

    // nk_console_checkbox()
    nk_bool checkbox_value = nk_true;
    nk_console* checkbox = nk_console_checkbox(console, "Checkbox", &checkbox_value);
    assert(checkbox != NULL);

    // nk_console_spacing()
    nk_console_label(console, "Spacing:");
    nk_console* spacing = nk_console_spacing(console, 3);
    assert(spacing != NULL);

    // nk_console_progress()
    nk_size process_value = 20;
    nk_console* progress = nk_console_progress(console, "Progress Bar", &process_value, 100);
    assert(progress != NULL);

    // nk_console_input()
    int gamepad_number = 0;
    enum nk_gamepad_button gamepad_button = NK_GAMEPAD_BUTTON_LB;
    nk_console* input = nk_console_input(console, "Input Button", -1, &gamepad_number, &gamepad_button);
    assert(input != NULL);

    // nk_console_combobox()
    int value_combobox = 3;
    nk_console* combobox = nk_console_combobox(console, "ComboBox", "Fists;Chainsaw;Pistol;Shotgun;Chaingun", ';', &value_combobox);
    assert(combobox != NULL);

    // nk_console_property_int/float()
    int property_int_test = 10;
    float property_float_test = 0.5f;
    nk_console* property_int = nk_console_property_int(console, "Property Int", 10, &property_int_test, 30, 1, 1);
    nk_console* property_float = nk_console_property_float(console, "Property Float", 0.0f, &property_float_test, 2.0f, 0.1f, 1);
    assert(property_int != NULL);
    assert(property_float != NULL);

    // slider_float/int()
    float slider_float_test = 1.0f;
    int slider_int_test = 15;
    nk_console* slider_float = nk_console_slider_float(console, "Slider Float", 0.0f, &slider_float_test, 2.0f, 0.1f);
    nk_console* slider_int = nk_console_slider_int(console, "Slider Int", 0, &slider_int_test, 20, 1);
    assert(slider_float != NULL);
    assert(slider_int != NULL);

    // nk_console_textedit()
    static const int textedit_buffer_size = 256;
    static char textedit_buffer[256] = "brianwatling";
    nk_console* textedit = nk_console_textedit(console, "Textedit", textedit_buffer, textedit_buffer_size);
    assert(textedit != NULL);

    // nk_console_color()
    struct nk_colorf color_value = {0.31f, 1.0f, 0.48f, 1.0f};
    nk_console* color = nk_console_color(console, "Color", &color_value, NK_RGBA);
    assert(color != NULL);

    // nk_console_file()
    static const int file_path_buffer_size = 256;
    static char file_path_buffer[256] = "";
    nk_console* file = nk_console_file(console, "File", file_path_buffer, file_path_buffer_size);
    assert(file != NULL);

    // nk_console_image()
    pntr_image* image_value = pntr_load_image("resources/image.png");
    assert(image_value != NULL);
    nk_console* image = nk_console_image(console, pntr_image_nk(image_value));
    nk_console_set_height(image, image_value->height);
    assert(image != NULL);

    // nk_console_show_message()
    nk_console_show_message(console, "This is an info message");

    // nk_console_row()
    nk_console* row = nk_console_row_begin(console);
    assert(row != NULL);
    nk_console_label(row, "Row Column 1")->alignment = NK_TEXT_CENTERED;
    nk_console_label(row, "Row Column 2")->alignment = NK_TEXT_CENTERED;
    nk_console_row_end(row);

    // nk_console_selectable()
    nk_console* widget = nk_console_label(console, "Selectable");
    widget->selectable = nk_false;
    widget->disabled = nk_false;
    assert(nk_console_selectable(widget) == nk_false);
    widget->selectable = nk_true;
    assert(nk_console_selectable(widget) == nk_true);
    widget->disabled = nk_true;
    assert(nk_console_selectable(widget) == nk_false);
    widget->selectable = nk_true;
    widget->disabled = nk_false;
    widget->visible = nk_true;
    assert(nk_console_selectable(widget) == nk_true);
    widget->visible = nk_false;
    assert(nk_console_selectable(widget) == nk_false);

    // Create the screen buffer
    pntr_image* screen = pntr_new_image(300, 800);
    assert(screen != NULL);

    // nk_console_rener_window()
    nk_console_render_window(console, "nuklear_console_test", nk_rect(0, 0, (float)screen->width, (float)screen->height), NK_WINDOW_TITLE);

    // Draw the nuklear context on the screen.
    pntr_draw_nuklear(screen, ctx);

    // Save the output test image.
    pntr_save_image(screen, "nuklear_console_test.png");

    // Unload
    nk_console_free(console);
    pntr_unload_nuklear(ctx);
    pntr_unload_image(image_value);
    pntr_unload_image(screen);
    pntr_unload_font(font);

    printf("nuklear_console_test: Tests passed.\n");

    return 0;
}
