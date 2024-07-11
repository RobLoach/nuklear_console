# nuklear_console

Console-like user interface for [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) with [gamepad](https://github.com/robloach/nuklear_gamepad), keyboard, and mouse support.

![nuklear_console_demo Screenshot](demo/common/nuklear_console_demo.gif)

## Usage

``` c
#define NK_IMPLEMENTATION
#include "nukear.h"

// Gamepad support optional https://github.com/robloach/nuklear_gamepad
#define NK_GAMEPAD_IMPLEMENTATION
#include "nuklear_gamepad.h"

#define NK_CONSOLE_IMPLEMENTATION
#include "nuklear_console.h"

int main() {
    // Set up the console within the Nuklear context
    nk_console* console = nk_console_init(ctx);

    // Add some widgets
    nk_console_button(console, "New Game");
    nk_console* options = nk_console_button(console, "Options");
    {
        nk_console_button(options, "Some cool option!");
        nk_console_button(options, "Option #2");
        nk_console_button_onclick(options, "Back", nk_console_button_back);
    }
    nk_console_button(console, "Load Game");
    nk_console_button(console, "Save Game");

    // Render the console in a window
    nk_begin();
        nk_console_render(console);
    nk_end();

    // Clean it up
    nk_console_free(console);

    return 0;
}
```

## Widgets

- Button
- Checkbox
- Combobox
- Label
- Property
- Slider
- Tooltip
- Row
- TextEdit

## API

``` c
// Console
nk_console* nk_console_init(struct nk_context* context);
void nk_console_free(nk_console* console);
void nk_console_render(nk_console* console);

// Widgets
nk_console* nk_console_button(nk_console* parent, const char* text);
nk_console* nk_console_checkbox(nk_console* parent, const char* text, nk_bool* active);
nk_console* nk_console_combobox(nk_console* parent, const char* label, const char *items_separated_by_separator, int separator, int* selected);
nk_console* nk_console_label(nk_console* parent, const char* text);
nk_console* nk_console_progress(nk_console* parent, const char* text, nk_size* current, nk_size max);
nk_console* nk_console_property_int(nk_console* parent, const char* label, int min, int *val, int max, int step, float inc_per_pixel);
nk_console* nk_console_property_float(nk_console* parent, const char* label, float min, float *val, float max, float step, float inc_per_pixel);
nk_console* nk_console_slider_int(nk_console* parent, const char* label, int min, int* val, int max, int step);
nk_console* nk_console_slider_float(nk_console* parent, const char* label, float min, float* val, float max, float step);
nk_console* nk_console_row_begin(nk_console* parent);
void nk_console_row_end(nk_console* console);
nk_console* nk_console_textedit(nk_console* parent, const char* label, char* buffer, int buffer_size);
void nk_console_show_message(nk_console* console, const char* text);

// Utilities
void nk_console_button_back(nk_console* button);
nk_console* nk_console_button_onclick(nk_console* parent, const char* text, void (*onclick)(struct nk_console*));
nk_console* nk_console_get_top(nk_console* widget);
int nk_console_get_widget_index(nk_console* widget);
void nk_console_check_tooltip(nk_console* console);
void nk_console_check_up_down(nk_console* widget, struct nk_rect bounds);
nk_bool nk_console_is_active_widget(nk_console* widget);
void nk_console_set_active_parent(nk_console* new_parent);
void nk_console_set_active_widget(nk_console* widget);
```

## Dependencies

- C99+
- [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear)
- [c-vector](https://github.com/eteran/c-vector/)
- [nuklear_gamepad](https://github.com/robloach/nuklear_gamepad) (optional)

## License

[MIT](LICENSE)
