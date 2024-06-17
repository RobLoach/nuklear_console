#ifndef NK_CONSOLE_TEXTEDIT_H__
#define NK_CONSOLE_TEXTEDIT_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_textedit_data {
    nk_console_button_data button; // Inherited from button.
    const char* buffer;
    int buffer_size;
    nk_bool shift;
} nk_console_textedit_data;

NK_API nk_console* nk_console_textedit(nk_console* parent, const char* label, const char* buffer, int buffer_size);
NK_API struct nk_rect nk_console_textedit_render(nk_console* console);
NK_API void nk_console_textedit_button_click(nk_console* button);
NK_API void nk_console_textedit_button_main_click(nk_console* button);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TEXTEDIT_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_TEXTEDIT_IMPLEMENTATION_ONCE
#define NK_CONSOLE_TEXTEDIT_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Handle the click event for textedit's children items.
 */
NK_API void nk_console_textedit_button_back_click(nk_console* button) {
    // Go back
    nk_console_button_back(button);

    // Clear out the on-screen keyboard
    nk_console* enter_textedit_button = button->parent;
    nk_console_free_children(enter_textedit_button);
}

/**
 * Handle the click event for the main button for the textedit.
 *
 * @see nk_console_textedit
 * @internal
 */
NK_API void nk_console_textedit_button_main_click(nk_console* button) {
    if (button == NULL || button->data == NULL) {
        return;
    }
    nk_console_free_children(button);

    // TODO: Create the on-screen keyboard.
    nk_console_textedit_data* data = (nk_console_textedit_data*)button->data;

    nk_console* row = nk_console_row_begin(button);
        nk_console_button(row, data->shift ? "A" : "a");
        nk_console_button(row, data->shift ? "B" : "b");
        nk_console_button(row, data->shift ? "C" : "c");
        nk_console_button(row, data->shift ? "D" : "d");
        nk_console_button(row, data->shift ? "E" : "e");
        nk_console_button(row, data->shift ? "F" : "f");
        nk_console_button(row, data->shift ? "G" : "g");
    nk_console_row_end(row);

    row = nk_console_row_begin(button);
        nk_console_button(row, data->shift ? "H" : "h");
        nk_console_button(row, data->shift ? "I" : "i");
        nk_console_button(row, data->shift ? "J" : "j");
        nk_console_button(row, data->shift ? "K" : "k");
        nk_console_button(row, data->shift ? "L" : "l");
        nk_console_button(row, data->shift ? "M" : "m");
        nk_console_button(row, data->shift ? "N" : "n");
    nk_console_row_end(row);

    row = nk_console_row_begin(button);
        nk_console_button(row, data->shift ? "O" : "o");
        nk_console_button(row, data->shift ? "P" : "p");
        nk_console_button(row, data->shift ? "Q" : "q");
        nk_console_button(row, data->shift ? "R" : "r");
        nk_console_button(row, data->shift ? "S" : "s");
        nk_console_button(row, data->shift ? "T" : "t");
        nk_console_button(row, data->shift ? "U" : "u");
    nk_console_row_end(row);

    row = nk_console_row_begin(button);
        nk_console_button(row, data->shift ? "V" : "v");
        nk_console_button(row, data->shift ? "W" : "w");
        nk_console_button(row, data->shift ? "X" : "x");
        nk_console_button(row, data->shift ? "Y" : "y");
        nk_console_button(row, data->shift ? "Z" : "z");
        nk_console_button(row, ".");
        nk_console_button(row, " ");
    nk_console_row_end(row);

    row = nk_console_row_begin(button);
        nk_console_button(row, data->shift ? "!" : "1");
        nk_console_button(row, data->shift ? "@" : "2");
        nk_console_button(row, data->shift ? "#" : "3");
        nk_console_button(row, data->shift ? "$" : "4");
        nk_console_button(row, data->shift ? "%" : "5");
        nk_console_button(row, data->shift ? "^" : "6");
        nk_console_button(row, data->shift ? "&" : "7");
        nk_console_button(row, data->shift ? "*" : "8");
        nk_console_button(row, data->shift ? "(" : "9");
        nk_console_button(row, data->shift ? ")" : "0");
    nk_console_row_end(row);

    nk_console_button_onclick(button, "Back", nk_console_textedit_button_back_click);

    nk_console_set_active_parent(button);
}

NK_API nk_console* nk_console_textedit(nk_console* parent, const char* label, const char* buffer, int buffer_size) {
    // Create the widget data.
    nk_handle unused = {0};
    nk_console_textedit_data* data = (nk_console_textedit_data*)NK_CONSOLE_MALLOC(unused, NULL, sizeof(nk_console_textedit_data));
    nk_zero(data, sizeof(nk_console_textedit_data));

    nk_console* textedit = nk_console_label(parent, label);
    textedit->type = NK_CONSOLE_COMBOBOX;
    textedit->selectable = nk_true;
    data->buffer = buffer;
    data->buffer_size = buffer_size;
    textedit->columns = 2;
    textedit->render = nk_console_textedit_render;
    textedit->data = data;

    nk_console_button_set_onclick(textedit, nk_console_textedit_button_main_click);

    return textedit;
}

NK_API struct nk_rect nk_console_textedit_render(nk_console* console) {
    nk_console_textedit_data* data = (nk_console_textedit_data*)console->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console* top = nk_console_get_top(console);
    int desired_columns = nk_strlen(console->label) > 0 ? console->columns : console->columns - 1;
    if (desired_columns > 0) {
        nk_layout_row_dynamic(console->context, 0, desired_columns);
    }

    // Display the label
    if (nk_strlen(console->label) > 0) {
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_begin(console->context);
        }
        nk_label(console->context, console->label, NK_TEXT_LEFT);
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_end(console->context);
        }
    }

    // Display the mocked textedit button
    int swap_columns = console->columns;
    console->columns = 0;
    const char* swap_label = console->label;
    int swap_label_length = console->label_length;

    console->label = data->buffer;
    console->label_length = console->label == NULL ? 0 : nk_strlen(console->label);
    if (console->label_length > 10) {
        console->label_length = 10;
    }

    struct nk_rect widget_bounds = nk_console_button_render(console);
    console->columns = swap_columns;

    console->label = swap_label;
    console->label_length = swap_label_length;

    return widget_bounds;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_TEXTEDIT_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
