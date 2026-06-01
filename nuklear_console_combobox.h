#ifndef NK_CONSOLE_COMBOBOX_H__
#define NK_CONSOLE_COMBOBOX_H__

/**
 * Data for Combobox widgets.
 */
typedef struct nk_console_combobox_data {
    nk_console_button_data button; // Inherited from button.
    const char* label;
    const char* items_separated_by_separator;
    int separator;
    int* selected;
    int count;
} nk_console_combobox_data;

#if defined(__cplusplus)
extern "C" {
#endif

/** Add a combobox widget with pipe-separated items. @return The new combobox widget. */
NK_API nk_console* nk_console_combobox(nk_console* parent, const char* label, const char* items_separated_by_separator, int separator, int* selected);
/** Render the combobox widget. @return The bounding rect. */
NK_API struct nk_rect nk_console_combobox_render(nk_console* console);
/** Internal: handle a click on a combobox option button. */
NK_API void nk_console_combobox_button_click(nk_console* button, void* user_data);
/** Internal: handle a click on the combobox's main (label) button. */
NK_API void nk_console_combobox_button_main_click(nk_console* button, void* user_data);
/** Replace the items and selected pointer on an existing combobox widget. */
NK_API void nk_console_combobox_update(nk_console* combobox, const char* label, const char* items_separated_by_separator, int separator, int* selected);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_COMBOBOX_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_COMBOBOX_IMPLEMENTATION_ONCE
#define NK_CONSOLE_COMBOBOX_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Handle the click event for combobox's children items.
 */
NK_API void nk_console_combobox_button_click(nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    nk_console* combobox = button->parent;
    nk_console_combobox_data* data = (nk_console_combobox_data*)combobox->data;

    // Find which option was selected.
    int selected = nk_console_get_widget_index(button);
    if (selected <= 0 || selected >= (int)cvector_size(combobox->children)) {
        nk_console_button_back(button, NULL);
        return;
    }

    // Update the active selected value.
    if (data->selected != NULL) {
        *data->selected = selected - 1;
    }

    // Change the combobox text that's displayed.
    combobox->label = button->label;
    combobox->label_length = button->label_length;

    // Go back
    nk_console_button_back(button, NULL);

    // Invoke the onchange callback.
    nk_console_trigger_event(combobox, NK_CONSOLE_EVENT_CHANGED);
}

/**
 * Handle the click event for the main button for the combobox.
 *
 * @see nk_console_combobox
 * @internal
 */
NK_API void nk_console_combobox_button_main_click(nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    nk_console_combobox_data* data = (nk_console_combobox_data*)button->data;
    int selected = data->selected == NULL ? 0 : *data->selected;
    if (button->children != NULL) {
        if ((int)cvector_size(button->children) > selected + 1) {
            nk_console_set_active_widget(button->children[selected + 1]);
        }
    }

    // Switch to show all the children.
    nk_console_set_active_parent(button);
}

/**
 * Update the combobox with new label, items, separator, and selected value.
 */
NK_API void nk_console_combobox_update(nk_console* combobox, const char* label, const char* items_separated_by_separator, int separator, int* selected) {
    if (!combobox || !combobox->data) return;
    nk_console_combobox_data* data = (nk_console_combobox_data*)combobox->data;

    // Update data fields
    data->label = label;
    data->items_separated_by_separator = items_separated_by_separator;
    data->separator = separator;
    data->selected = selected;

    // Update combobox label
    combobox->label = label;
    combobox->label_length = nk_strlen(label);

    // Clear out any existing children
    nk_console_free_children(combobox);

    // Add the Back button
    nk_console* backbutton = nk_console_button_onclick(combobox, label, &nk_console_combobox_button_click);
    nk_console_button_set_symbol(backbutton, NK_SYMBOL_TRIANGLE_UP);

    // NULL items: clamp selected and leave only the back button.
    if (items_separated_by_separator == NULL) {
        if (selected != NULL) {
            *selected = 0;
        }
        return;
    }

    // Add new items as children
    const char* button_text_start = items_separated_by_separator;
    int text_length = 0;
    for (int i = 0; items_separated_by_separator[i] != 0; i++) {
        text_length++;
        if (items_separated_by_separator[i] == (char)separator) {
            nk_console_button_onclick(combobox, button_text_start, &nk_console_combobox_button_click)
                ->label_length = text_length - 1;
            text_length = 0;
            button_text_start = items_separated_by_separator + i + 1;
        }
    }

    // Add the last item
    nk_console_button_onclick(combobox, button_text_start, &nk_console_combobox_button_click)
        ->label_length = text_length;

    // Update selected value and combobox label
    if (selected != NULL) {
        if (*selected < 0) {
            *selected = 0;
        }
        else if (*selected >= (int)cvector_size(combobox->children) - 1) {
            *selected = (int)cvector_size(combobox->children) - 2;
        }
        combobox->label = combobox->children[*selected + 1]->label;
        combobox->label_length = combobox->children[*selected + 1]->label_length;
    }
}

NK_API nk_console* nk_console_combobox(nk_console* parent, const char* label, const char* items_separated_by_separator, int separator, int* selected) {
    // Create the widget data.
    nk_console_combobox_data* data = (nk_console_combobox_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_combobox_data));
    nk_zero(data, sizeof(nk_console_combobox_data));

    // Set up the combobox
    nk_console* combobox = nk_console_label(parent, label);
    combobox->type = NK_CONSOLE_COMBOBOX;
    combobox->flags |= NK_CONSOLE_FLAG_SELECTABLE;
    combobox->columns = label != NULL ? 2 : 1;
    combobox->render = nk_console_combobox_render;
    combobox->data = data;

    nk_console_button_set_symbol(combobox, NK_SYMBOL_TRIANGLE_DOWN);
    nk_console_add_event(combobox, NK_CONSOLE_EVENT_CLICKED, nk_console_combobox_button_main_click);

    // Set up the combobox items and label
    nk_console_combobox_update(combobox, label, items_separated_by_separator, separator, selected);

    return combobox;
}

NK_API struct nk_rect nk_console_combobox_render(nk_console* console) {
    nk_console_combobox_data* data = (nk_console_combobox_data*)console->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console* top = nk_console_get_top(console);

    nk_console_layout_widget(console);

    // Allow changing the value with left/right
    if (NK_FLAG_DISABLED(console->flags, NK_CONSOLE_FLAG_DISABLED) && nk_console_is_active_widget(console)) {
        nk_console_top_data* top_data = (nk_console_top_data*)top->data;
        if (NK_FLAG_DISABLED(top_data->state, NK_CONSOLE_TOP_FLAG_INPUT_PROCESSED) && data->selected != NULL && console->children != NULL) {
            nk_bool changed = nk_false;
            if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT) && *data->selected > 0) {
                *data->selected = *data->selected - 1;
                changed = nk_true;
            }
            else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT) && *data->selected < (int)cvector_size(console->children) - 2) {
                *data->selected = *data->selected + 1;
                changed = nk_true;
            }

            if (changed) {
                top_data->state |= NK_CONSOLE_TOP_FLAG_INPUT_PROCESSED;
                console->label = console->children[*data->selected + 1]->label;
                console->label_length = console->children[*data->selected + 1]->label_length;
                nk_console_trigger_event(console, NK_CONSOLE_EVENT_CHANGED);
            }
        }
    }

    // Display the label
    if (nk_strlen(data->label) > 0) {
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_begin(console->ctx);
        }
        nk_label(console->ctx, data->label, NK_TEXT_LEFT);
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_end(console->ctx);
        }
    }

    // Display the mocked combobox button
    int swap_columns = console->columns;
    console->columns = 0;
    // console->type = NK_CONSOLE_BUTTON;
    if (nk_console_is_active_widget(console)) {
        nk_console_button_set_symbol(console, NK_SYMBOL_TRIANGLE_DOWN);
    }
    else {
        nk_console_button_set_symbol(console, NK_SYMBOL_NONE);
    }

    struct nk_rect widget_bounds = nk_console_button_render(console);
    console->columns = swap_columns;
    return widget_bounds;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_COMBOBOX_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
