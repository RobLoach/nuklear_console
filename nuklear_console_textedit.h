#ifndef NK_CONSOLE_TEXTEDIT_H__
#define NK_CONSOLE_TEXTEDIT_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_textedit_data {
    nk_console_button_data button; // Inherited from button.
    char* buffer;
    int buffer_size;
    nk_bool shift;
} nk_console_textedit_data;

NK_API nk_console* nk_console_textedit(nk_console* parent, const char* label, char* buffer, int buffer_size);
NK_API struct nk_rect nk_console_textedit_render(nk_console* console);
NK_API void nk_console_textedit_button_click(nk_console* button);
NK_API void nk_console_textedit_button_main_click(nk_console* button);
NK_API void nk_console_textedit_button_back_click(nk_console* button);

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
    // Make sure we're not going back to a Row.
    if (button->parent->type == NK_CONSOLE_ROW) {
        button = button->parent;
    }

    nk_console_button_back(button);

    // Clear out the on-screen keyboard
    nk_console* enter_textedit_button = button->parent;
    nk_console_free_children(enter_textedit_button);
}

#include <stdio.h>
NK_API void nk_console_textedit_key_click(nk_console* key) {
    // Find the textedit widget, and make sure it's not a row.
    nk_console* textedit = key->parent;
    if (textedit->type == NK_CONSOLE_ROW) {
        textedit = textedit->parent;
    }

    // Get the textedit data.
    nk_console_textedit_data* data = (nk_console_textedit_data*)textedit->data;

    // Handle the key press
    enum nk_symbol_type symbol = nk_console_button_get_symbol(key);
    switch (symbol) {
        case NK_SYMBOL_CIRCLE_SOLID:
        case NK_SYMBOL_CIRCLE_OUTLINE:
            data->shift = !data->shift;
            nk_console_button_set_symbol(key, symbol == NK_SYMBOL_CIRCLE_SOLID ? NK_SYMBOL_CIRCLE_OUTLINE : NK_SYMBOL_CIRCLE_SOLID);



            // Replace all labels of the parent buttons with shifted characters.
            for (size_t x = 0; x < cvector_size(textedit->children); ++x) {
                nk_console* child = textedit->children[x];
                if (child->type == NK_CONSOLE_ROW) {
                    nk_console_row* row = (nk_console_row*)child;
                    for (size_t i = 0; i < cvector_size(row->data.children); ++i) {
                        nk_console* activeButton = row->data.children[i].console;
                        if (activeButton->type == NK_CONSOLE_BUTTON) {
                            const char* label = activeButton->label;
                            if (label != NULL && nk_strlen(label) == 1) {
                                char old_char = label[0];
                                switch (old_char) {
                                    case 'a': label = "A"; break;
                                    case 'b': label = "B"; break;
                                    case 'c': label = "C"; break;
                                    case 'd': label = "D"; break;
                                    case 'e': label = "E"; break;
                                    case 'f': label = "F"; break;
                                    case 'g': label = "G"; break;
                                    case 'h': label = "H"; break;
                                    case 'i': label = "I"; break;
                                    case 'j': label = "J"; break;
                                    case 'k': label = "K"; break;
                                    case 'l': label = "L"; break;
                                    case 'm': label = "M"; break;
                                    case 'n': label = "N"; break;
                                    case 'o': label = "O"; break;
                                    case 'p': label = "P"; break;
                                    case 'q': label = "Q"; break;
                                    case 'r': label = "R"; break;
                                    case 's': label = "S"; break;
                                    case 't': label = "T"; break;
                                    case 'u': label = "U"; break;
                                    case 'v': label = "V"; break;
                                    case 'w': label = "W"; break;
                                    case 'x': label = "X"; break;
                                    case 'y': label = "Y"; break;
                                    case 'z': label = "Z"; break;
                                    case 'A': label = "a"; break;
                                    case 'B': label = "b"; break;
                                    case 'C': label = "c"; break;
                                    case 'D': label = "d"; break;
                                    case 'E': label = "e"; break;
                                    case 'F': label = "f"; break;
                                    case 'G': label = "g"; break;
                                    case 'H': label = "h"; break;
                                    case 'I': label = "i"; break;
                                    case 'J': label = "j"; break;
                                    case 'K': label = "k"; break;
                                    case 'L': label = "l"; break;
                                    case 'M': label = "m"; break;
                                    case 'N': label = "n"; break;
                                    case 'O': label = "o"; break;
                                    case 'P': label = "p"; break;
                                    case 'Q': label = "q"; break;
                                    case 'R': label = "r"; break;
                                    case 'S': label = "s"; break;
                                    case 'T': label = "t"; break;
                                    case 'U': label = "u"; break;
                                    case 'V': label = "v"; break;
                                    case 'W': label = "w"; break;
                                    case 'X': label = "x"; break;
                                    case 'Y': label = "y"; break;
                                    case 'Z': label = "z"; break;

                                    // Symbols
                                    case '>': label = "."; break;
                                    case '.': label = ">"; break;
                                    case '<': label = ","; break;
                                    case ',': label = "<"; break;

                                    // Numbers
                                    case '1': label = "!"; break;
                                    case '2': label = "@"; break;
                                    case '3': label = "#"; break;
                                    case '4': label = "$"; break;
                                    case '5': label = "%"; break;
                                    case '6': label = "^"; break;
                                    case '7': label = "&"; break;
                                    case '8': label = "*"; break;
                                    case '9': label = "("; break;
                                    case '0': label = ")"; break;
                                    case '!': label = "1"; break;
                                    case '@': label = "2"; break;
                                    case '#': label = "3"; break;
                                    case '$': label = "4"; break;
                                    case '%': label = "5"; break;
                                    case '^': label = "6"; break;
                                    case '&': label = "7"; break;
                                    case '*': label = "8"; break;
                                    case '(': label = "9"; break;
                                    case ')': label = "0"; break;
                                }
                                nk_console_set_label(activeButton, label, 1);
                            }
                        }
                    }
                }
            }
            return;
        case NK_SYMBOL_TRIANGLE_LEFT:
            {
                int len = nk_strlen(data->buffer);
                if (len > 0) {
                    data->buffer[len - 1] = '\0';
                }
            }
            return;
        // case NK_SYMBOL_UNDERSCORE:
        //     {
        //         int len = nk_strlen(data->buffer);
        //         if (len < data->buffer_size - 1) {
        //             data->buffer[len] = ' ';
        //             data->buffer[len + 1] = '\0';
        //         }
        //     }
        //     return;
        // case NK_SYMBOL_TRIANGLE_LEFT:
        //     if (key->context->current->edit.cursor > 0) {
        //         key->context->current->edit.cursor--;
        //     }
        //     return;
        // case NK_SYMBOL_TRIANGLE_RIGHT:
        //     if (key->context->current->edit.cursor < nk_strlen(data->buffer)) {
        //         key->context->current->edit.cursor++;
        //     }
        //     return;
    }

    // Add the character to the buffer.
    int len = nk_strlen(data->buffer);
    if (len < data->buffer_size - 1) {
        data->buffer[len] = key->label[0];
        data->buffer[len + 1] = '\0';
    }
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

    // Make sure there aren't any children.
    nk_console_free_children(button);

    // TODO: Create the on-screen keyboard.
    nk_console_textedit_data* data = (nk_console_textedit_data*)button->data;
    nk_console* key;

    nk_console_textedit_text(button);

    nk_console* row = nk_console_row_begin(button);
        nk_console_button_onclick(row, data->shift ? "!" : "1", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "@" : "2", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "#" : "3", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "$" : "4", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "%" : "5", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "^" : "6", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "&" : "7", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "*" : "8", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "(" : "9", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? ")" : "0", nk_console_textedit_key_click);
    nk_console_row_end(row);

    row = nk_console_row_begin(button);
        nk_console_button_onclick(row, data->shift ? "Q" : "q", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "W" : "w", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "E" : "e", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "R" : "r", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "T" : "t", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "Y" : "y", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "U" : "u", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "I" : "i", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "O" : "o", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "P" : "p", nk_console_textedit_key_click);
    nk_console_row_end(row);
    row = nk_console_row_begin(button);
        nk_console_button_onclick(row, data->shift ? "A" : "a", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "S" : "s", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "D" : "d", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "F" : "f", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "G" : "g", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "H" : "h", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "J" : "j", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "K" : "k", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "L" : "l", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? ">" : ".", nk_console_textedit_key_click);
    nk_console_row_end(row);
    row = nk_console_row_begin(button);
        key = nk_console_button_onclick(row, "", nk_console_textedit_key_click);
        if (data->shift) {
            nk_console_button_set_symbol(key, NK_SYMBOL_CIRCLE_SOLID);
        }
        else {
            nk_console_button_set_symbol(key, NK_SYMBOL_CIRCLE_OUTLINE);
        }
        nk_console_button_onclick(row, data->shift ? "Z" : "z", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "X" : "x", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "C" : "c", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "V" : "v", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "B" : "b", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "N" : "n", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "M" : "m", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "<" : ",", nk_console_textedit_key_click);
        key = nk_console_button_onclick(row, "", nk_console_textedit_key_click); // Backspace
            nk_console_button_set_symbol(key, NK_SYMBOL_TRIANGLE_LEFT);
    nk_console_row_end(row);

    row = nk_console_row_begin(button);
        nk_console_button_onclick(row, " ", nk_console_textedit_key_click); // Space
    nk_console_row_end(row);

    row = nk_console_row_begin(button);
        nk_console_button_onclick(row, "Enter", nk_console_textedit_button_back_click);
    nk_console_row_end(row);

    nk_console_set_active_parent(button);
}

NK_API nk_console* nk_console_textedit(nk_console* parent, const char* label, char* buffer, int buffer_size) {
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
