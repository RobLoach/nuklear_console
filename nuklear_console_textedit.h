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

/**
 * Adds a textedit on-screen keyboard widget to the console given parent.
 *
 * @param parent The parent widget to add the textedit to.
 * @param buffer The buffer to store the textedit data.
 * @param buffer_size The size of the given buffer.
 * @return The created textedit widget.
 */
NK_API nk_console* nk_console_textedit(nk_console* parent, const char* label, char* buffer, int buffer_size);
NK_API struct nk_rect nk_console_textedit_render(nk_console* console);
NK_API void nk_console_textedit_button_main_click(nk_console* button, void* user_data);
NK_API void nk_console_textedit_button_back_click(nk_console* button, void* user_data);
NK_API void nk_console_textedit_key_click(nk_console* key, void* user_data);

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
NK_API void nk_console_textedit_button_back_click(nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    // Make sure we're not going back to a row.
    if (button->parent->type == NK_CONSOLE_ROW) {
        button = button->parent;
    }

    // Invoke the back button behavior on the button.
    nk_console_button_back(button, NULL);

    // Clear out the on-screen keyboard keys
    nk_console* enter_textedit_button = button->parent;
    nk_console_free_children(enter_textedit_button);
}

NK_API void nk_console_textedit_key_click(nk_console* key, void* user_data) {
    NK_UNUSED(user_data);
    if (key == NULL) {
        return;
    }

    // Find the textedit widget, and make sure it's not the row.
    nk_console* textedit = key->parent;
    if (textedit->type == NK_CONSOLE_ROW) {
        textedit = textedit->parent;
    }
    NK_ASSERT(textedit != NULL && textedit->type == NK_CONSOLE_TEXTEDIT);

    // Get the textedit data.
    nk_console_textedit_data* data = (nk_console_textedit_data*)textedit->data;

    // Handle the key press
    enum nk_symbol_type symbol = nk_console_button_get_symbol(key);
    switch (symbol) {
        // Shift
        case NK_SYMBOL_TRIANGLE_UP:
        case NK_SYMBOL_TRIANGLE_UP_OUTLINE: {
            data->shift = !data->shift;
            nk_console_button_set_symbol(key, symbol == NK_SYMBOL_TRIANGLE_UP ? NK_SYMBOL_TRIANGLE_UP_OUTLINE : NK_SYMBOL_TRIANGLE_UP);

            // Replace all labels of the parent buttons with shifted characters.
            // TODO: Clean up shifting the key labels so that it's more dynamic.
            int textedit_children_size = (int)cvector_size(textedit->children);
            for (int x = 0; x < textedit_children_size; ++x) {
                nk_console* child = textedit->children[x];
                if (child->type == NK_CONSOLE_ROW) {
                    int child_children_size = (int)cvector_size(child->children);
                    for (int i = 0; i < child_children_size; ++i) {
                        nk_console* activeButton = child->children[i];
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
        }
        break;

        // Backspace
        case NK_SYMBOL_TRIANGLE_LEFT:
            {
                int len = nk_strlen(data->buffer);
                if (len > 0) {
                    data->buffer[len - 1] = '\0';
                }
            }
            break;

        // Space
        case NK_SYMBOL_RECT_SOLID:
            {
                int len = nk_strlen(data->buffer);
                if (len < data->buffer_size - 1) {
                    data->buffer[len] = ' ';
                    data->buffer[len + 1] = '\0';
                }
            }
            break;

        // Any key character
        case NK_SYMBOL_NONE:
            {
                // Add the character to the buffer.
                int len = nk_strlen(data->buffer);
                if (len < data->buffer_size - 1) {
                    data->buffer[len] = key->label[0];
                    data->buffer[len + 1] = '\0';
                }
            }
            break;

        default:
            break;
    }
}

/**
 * Handle the click event for the main button for the textedit.
 *
 * @see nk_console_textedit
 * @internal
 */
NK_API void nk_console_textedit_button_main_click(nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    if (button == NULL || button->data == NULL) {
        return;
    }

    // Make sure there aren't any children to recreate the keyboard.
    nk_console_free_children(button);

    nk_console_textedit_data* data = (nk_console_textedit_data*)button->data;
    nk_console* key;

    nk_console_textedit_text(button);

    // TODO: Add option for UTF-8 keys with nk_glyph.

    // First row: 1 - 0
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

    // Second row: Q - P
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

    // Third row: A - L
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

    // Fourth row: Z - M
    row = nk_console_row_begin(button);
        key = nk_console_button_onclick(row, NULL, nk_console_textedit_key_click);
        if (data->shift) {
            nk_console_button_set_symbol(key, NK_SYMBOL_TRIANGLE_UP);
        }
        else {
            nk_console_button_set_symbol(key, NK_SYMBOL_TRIANGLE_UP_OUTLINE);
        }
        nk_console_button_onclick(row, data->shift ? "Z" : "z", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "X" : "x", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "C" : "c", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "V" : "v", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "B" : "b", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "N" : "n", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "M" : "m", nk_console_textedit_key_click);
        nk_console_button_onclick(row, data->shift ? "<" : ",", nk_console_textedit_key_click);
        key = nk_console_button_onclick(row, NULL, nk_console_textedit_key_click); // Backspace
            nk_console_button_set_symbol(key, NK_SYMBOL_TRIANGLE_LEFT);
    nk_console_row_end(row);

    // Fifth row: Space and Back
    row = nk_console_row_begin(button);
    {
        // Space
        key = nk_console_button_onclick(row, NULL, nk_console_textedit_key_click); // Space
        key->columns = 3;
        nk_console_button_set_symbol(key, NK_SYMBOL_RECT_SOLID);

        // Back
        // TODO: textedit: Replace "Back" with a RETURN symbol?
        key = nk_console_button_onclick(row, "Back", nk_console_textedit_button_back_click);
        key->columns = 1;
    }
    nk_console_row_end(row);

    // Make the onscreen keyboard the active widget.
    nk_console_set_active_parent(button);
}

NK_API nk_console* nk_console_textedit(nk_console* parent, const char* label, char* buffer, int buffer_size) {
    // Create the widget data.
    nk_console_textedit_data* data = (nk_console_textedit_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_textedit_data));
    nk_zero(data, sizeof(nk_console_textedit_data));

    // Create the textedit widget.
    nk_console* textedit = nk_console_label(parent, label);
    textedit->type = NK_CONSOLE_TEXTEDIT;
    textedit->selectable = nk_true;
    data->buffer = buffer;
    data->buffer_size = buffer_size;
    textedit->columns = label != NULL ? 2 : 1;
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

    nk_console_layout_widget(console);

    // Display the label
    if (console->label != NULL && nk_strlen(console->label) > 0) {
        nk_bool active = nk_console_is_active_widget(console);
        if (!active) {
            nk_widget_disable_begin(console->ctx);
        }
        nk_label(console->ctx, console->label, NK_TEXT_LEFT);
        if (!active) {
            nk_widget_disable_end(console->ctx);
        }
    }

    // Display the mocked textedit button
    int swap_columns = console->columns;
    console->columns = 0; // We use 0 as w'ere not making a new row.
    const char* swap_label = console->label;
    int swap_label_length = console->label_length;

    console->label = data->buffer;
    console->label_length = console->label == NULL ? 0 : nk_strlen(console->label);
    if (console->label_length > 10) {
        console->label_length = 10;
    }

    struct nk_rect widget_bounds = nk_console_button_render(console);

    // Restore the previous values for the textedit widget.
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
