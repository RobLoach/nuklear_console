#ifndef NK_CONSOLE_TEXTEDIT_H__
#define NK_CONSOLE_TEXTEDIT_H__

#ifndef NK_CONSOLE_TEXTEDIT_MASKED_LENGTH
/**
 * How long masked text should be when displayed.
 */
#define NK_CONSOLE_TEXTEDIT_MASKED_LENGTH 8
#endif

#ifndef NK_CONSOLE_TEXTEDIT_PREVIEW_LENGTH
/**
 * Maximum number of characters shown in the textedit button preview.
 */
#define NK_CONSOLE_TEXTEDIT_PREVIEW_LENGTH 10
#endif

/**
 * A key entry in a keyboard layout row.
 * A row is a NULL-terminated array of these; the last entry has normal == NULL.
 */
typedef struct nk_console_textedit_key {
    const char* normal;   /**< Label when shift is off; NULL terminates a row. */
    const char* shifted;  /**< Label when shift is on; NULL means same as normal. */
} nk_console_textedit_key;

typedef struct nk_console_textedit_data {
    nk_console_button_data button; // Inherited from button.
    char* buffer;
    int buffer_size;
    nk_bool shift;
    nk_bool masked; // When true, displays '*' instead of the actual buffer content.
    nk_bool textedit_action; /** When true, uses the label as the button text and skips the left-side label. */
    nk_bool changed; /** Set when the buffer is edited; CHANGED event fires on back if true. */
    char masked_display[NK_CONSOLE_TEXTEDIT_MASKED_LENGTH + 4]; // Space for null-terminator and some protection.
    const nk_console_textedit_key** keyboard_layout; /**< NULL-terminated array of key rows; last row gets shift/backspace added. */
} nk_console_textedit_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Adds a textedit on-screen keyboard widget to the console given parent.
 *
 * @param parent The parent widget to add the textedit to.
 * @param label A string representing the identifier for the associated textedit widget.
 * @param buffer The buffer to store the textedit data.
 * @param buffer_size The size of the given buffer.
 * @return The created textedit widget.
 */
NK_API nk_console* nk_console_textedit(nk_console* parent, const char* label, char* buffer, int buffer_size);
NK_API nk_console* nk_console_textedit_masked(nk_console* parent, const char* label, char* buffer, int buffer_size);

/**
 * Creates a textedit action widget that shows a single full-width button. The label is used as the
 * button text before any text is entered; afterwards the button shows a preview of the entered text.
 * Clicking the button opens the on-screen keyboard, just like nk_console_textedit.
 *
 * @param parent The parent widget.
 * @param label The button label. For example: "Enter text". Pass NULL for the default text.
 * @param buffer The buffer to store the textedit data.
 * @param buffer_size The size of the given buffer.
 *
 * @return The new textedit action widget.
 */
NK_API nk_console* nk_console_textedit_action(nk_console* parent, const char* label, char* buffer, int buffer_size);
NK_API void nk_console_textedit_set_keyboard_layout(nk_console* textedit, const nk_console_textedit_key** layout);
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

/* Default ASCII keyboard layout rows (NULL-terminated; last row gets Shift+Backspace). */
static const nk_console_textedit_key nk_console_textedit_ascii_row0[] = {
    {"1","!"}, {"2","@"}, {"3","#"}, {"4","$"}, {"5","%"},
    {"6","^"}, {"7","&"}, {"8","*"}, {"9","("}, {"0",")"},
    {NULL, NULL}
};
static const nk_console_textedit_key nk_console_textedit_ascii_row1[] = {
    {"q","Q"}, {"w","W"}, {"e","E"}, {"r","R"}, {"t","T"},
    {"y","Y"}, {"u","U"}, {"i","I"}, {"o","O"}, {"p","P"},
    {NULL, NULL}
};
static const nk_console_textedit_key nk_console_textedit_ascii_row2[] = {
    {"a","A"}, {"s","S"}, {"d","D"}, {"f","F"}, {"g","G"},
    {"h","H"}, {"j","J"}, {"k","K"}, {"l","L"}, {".",">"},
    {NULL, NULL}
};
static const nk_console_textedit_key nk_console_textedit_ascii_row3[] = {
    {"z","Z"}, {"x","X"}, {"c","C"}, {"v","V"}, {"b","B"},
    {"n","N"}, {"m","M"}, {",","<"},
    {NULL, NULL}
};
static const nk_console_textedit_key* nk_console_textedit_layout_ascii[] = {
    nk_console_textedit_ascii_row0,
    nk_console_textedit_ascii_row1,
    nk_console_textedit_ascii_row2,
    nk_console_textedit_ascii_row3,
    NULL
};

/**
 * Frees all the children for the given textedit as an event.
 */
static void nk_console_textedit_free_children(nk_console* textedit, void* user_data) {
    NK_UNUSED(user_data);
    nk_console_free_children(textedit);
}

/**
 * Adds a post-render event to clear out the children when going back.
 *
 * This is done in post-render so that if there are any existing elements that are referencing the elements, it is cleared out afterwards.
 */
static void nk_console_textedit_text_event_back(struct nk_console* widget, void* user_data) {
    NK_UNUSED(user_data);
    nk_console_textedit_data* data = (nk_console_textedit_data*)widget->data;
    if (data != NULL && data->changed) {
        data->changed = nk_false;
        nk_console_trigger_event(widget, NK_CONSOLE_EVENT_CHANGED);
    }
    nk_console_add_event(widget, NK_CONSOLE_EVENT_POST_RENDER_ONCE, &nk_console_textedit_free_children);
}

/**
 * Handle the click event for textedit's children items.
 */
NK_API void nk_console_textedit_button_back_click(nk_console* button, void* user_data) {
    if (button == NULL) {
        return;
    }
    NK_UNUSED(user_data);

    // Make sure we're not going back to a row.
    if (button->parent->type == NK_CONSOLE_ROW) {
        button = button->parent;
    }

    // Invoke the back button behavior on the button.
    nk_console_button_back(button, NULL);
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

            // Update all key button labels using the stored layout table.
            const nk_console_textedit_key** layout = data->keyboard_layout;
            int textedit_children_size = (int)cvector_size(textedit->children);
            for (int x = 0; x < textedit_children_size; ++x) {
                nk_console* child = textedit->children[x];
                if (child->type == NK_CONSOLE_ROW) {
                    int child_children_size = (int)cvector_size(child->children);
                    for (int i = 0; i < child_children_size; ++i) {
                        nk_console* activeButton = child->children[i];
                        if (activeButton->type == NK_CONSOLE_BUTTON && activeButton->label != NULL) {
                            const char* lbl = activeButton->label;
                            nk_bool found = nk_false;
                            for (int r = 0; !found && layout[r] != NULL; r++) {
                                for (int k = 0; !found && layout[r][k].normal != NULL; k++) {
                                    const char* normal = layout[r][k].normal;
                                    const char* shifted = layout[r][k].shifted != NULL ? layout[r][k].shifted : normal;
                                    if (strcmp(lbl, normal) == 0 || strcmp(lbl, shifted) == 0) {
                                        const char* new_lbl = data->shift ? shifted : normal;
                                        nk_console_set_label(activeButton, new_lbl, nk_strlen(new_lbl));
                                        found = nk_true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } break;

        // Backspace
        case NK_SYMBOL_TRIANGLE_LEFT: {
            int len = nk_strlen(data->buffer);
            if (len > 0) {
                // Walk back past UTF-8 continuation bytes (0x80-0xBF) to erase the full codepoint.
                do {
                    len--;
                } while (len > 0 && ((unsigned char)data->buffer[len] & 0xC0) == 0x80);
                data->buffer[len] = '\0';
                data->changed = nk_true;
            }
        } break;

        // Space
        case NK_SYMBOL_RECT_SOLID: {
            int len = nk_strlen(data->buffer);
            if (len < data->buffer_size - 1) {
                data->buffer[len] = ' ';
                data->buffer[len + 1] = '\0';
                data->changed = nk_true;
            }
        } break;

        // Any key character
        case NK_SYMBOL_NONE: {
            // Append all bytes of the key label to support multi-byte UTF-8 glyphs.
            int len = nk_strlen(data->buffer);
            int key_len = nk_strlen(key->label);
            if (len + key_len < data->buffer_size) {
                NK_MEMCPY(data->buffer + len, key->label, (nk_size)(key_len + 1));
                data->changed = nk_true;
            }
        } break;

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
    data->changed = nk_false;
    nk_console* key;

    // Create the textedit_text widget, which is the input box.
    nk_console_textedit_text(button);

    // Build the keyboard from the layout table. Rows [0..N-2] are plain key rows;
    // the last row gets a Shift key prepended and a Backspace key appended.
    const nk_console_textedit_key** layout = data->keyboard_layout;
    int row_count = 0;
    while (layout[row_count] != NULL) row_count++;

    for (int r = 0; r < row_count; r++) {
        const nk_console_textedit_key* row_keys = layout[r];
        nk_bool is_last_row = (r == row_count - 1);

        nk_console* row = nk_console_row_begin(button);
        {
            if (is_last_row) {
                key = nk_console_button_onclick(row, NULL, &nk_console_textedit_key_click);
                nk_console_button_set_symbol(key, data->shift ? NK_SYMBOL_TRIANGLE_UP : NK_SYMBOL_TRIANGLE_UP_OUTLINE);
            }

            for (int k = 0; row_keys[k].normal != NULL; k++) {
                const char* shifted = row_keys[k].shifted != NULL ? row_keys[k].shifted : row_keys[k].normal;
                nk_console_button_onclick(row, data->shift ? shifted : row_keys[k].normal, &nk_console_textedit_key_click);
            }

            if (is_last_row) {
                key = nk_console_button_onclick(row, NULL, &nk_console_textedit_key_click);
                nk_console_button_set_symbol(key, NK_SYMBOL_TRIANGLE_LEFT);
            }
        }
        nk_console_row_end(row);
    }

    // Always append a Space + Back row.
    nk_console* row = nk_console_row_begin(button);
    {
        key = nk_console_button_onclick(row, NULL, &nk_console_textedit_key_click);
        key->columns = 3;
        nk_console_button_set_symbol(key, NK_SYMBOL_RECT_SOLID);

        key = nk_console_button_onclick(row, "Back", &nk_console_textedit_button_back_click);
        key->columns = 1;
    }
    nk_console_row_end(row);

    // Make the onscreen keyboard the active widget.
    nk_console_set_active_parent(button);
}

NK_API nk_console* nk_console_textedit(nk_console* parent, const char* label, char* buffer, int buffer_size) {
    // Create the widget data.
    nk_console_textedit_data* data = (nk_console_textedit_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_textedit_data));
    if (data == NULL) return NULL;
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

    // Initialize the mask with '*'.
    data->masked = nk_false;
    for (int i = 0; i < NK_CONSOLE_TEXTEDIT_MASKED_LENGTH; ++i) {
        data->masked_display[i] = '*';
    }
    data->masked_display[NK_CONSOLE_TEXTEDIT_MASKED_LENGTH] = '\0';
    data->keyboard_layout = nk_console_textedit_layout_ascii;

    nk_console_add_event(textedit, NK_CONSOLE_EVENT_CLICKED, &nk_console_textedit_button_main_click);
    nk_console_add_event(textedit, NK_CONSOLE_EVENT_BACK, &nk_console_textedit_text_event_back);

    return textedit;
}

NK_API nk_console* nk_console_textedit_masked(nk_console* parent, const char* label, char* buffer, int buffer_size) {
    nk_console* textedit = nk_console_textedit(parent, label, buffer, buffer_size);
    if (textedit == NULL) {
        return NULL;
    }

    nk_console_textedit_data* data = (nk_console_textedit_data*)textedit->data;
    data->masked = nk_true;
    return textedit;
}

NK_API nk_console* nk_console_textedit_action(nk_console* parent, const char* label, char* buffer, int buffer_size) {
    nk_console* textedit = nk_console_textedit(parent, label, buffer, buffer_size);
    if (textedit == NULL) {
        return NULL;
    }

    nk_console_textedit_data* data = (nk_console_textedit_data*)textedit->data;
    data->textedit_action = nk_true;
    textedit->columns = 1;
    return textedit;
}

NK_API void nk_console_textedit_set_keyboard_layout(nk_console* textedit, const nk_console_textedit_key** layout) {
    if (!textedit || !textedit->data) return;
    nk_console_textedit_data* data = (nk_console_textedit_data*)textedit->data;
    data->keyboard_layout = layout != NULL ? layout : nk_console_textedit_layout_ascii;
}

NK_API struct nk_rect nk_console_textedit_render(nk_console* console) {
    nk_console_textedit_data* data = (nk_console_textedit_data*)console->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(console);

    // With textedit_action mode, change the label to what was entered?
    if (!data->textedit_action && console->label != NULL && nk_strlen(console->label) > 0) {
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
    console->columns = 0; // We use 0 as we're not making a new row.
    const char* swap_label = console->label;
    int swap_label_length = console->label_length;

    // Display the label, which is the buffer.
    console->label = data->buffer;
    console->label_length = console->label == NULL ? 0 : nk_strlen(console->label);
    if (console->label_length > NK_CONSOLE_TEXTEDIT_PREVIEW_LENGTH) {
        console->label_length = NK_CONSOLE_TEXTEDIT_PREVIEW_LENGTH;
    }

    // Mask it, if needed.
    if (data->masked) {
        console->label = data->masked_display;
        console->label_length = NK_CONSOLE_TEXTEDIT_MASKED_LENGTH;
    }

    // In textedit_action mode, always use the widget label as the button text.
    if (data->textedit_action) {
        if (swap_label != NULL && swap_label[0] != '\0') {
            console->label = swap_label;
            console->label_length = swap_label_length;
        }
        else {
            console->label = "[Enter Text]";
            console->label_length = 0;
        }
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

#endif // NK_CONSOLE_TEXTEDIT_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
