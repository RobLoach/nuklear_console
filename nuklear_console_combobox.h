#ifndef NK_CONSOLE_COMBOBOX_H__
#define NK_CONSOLE_COMBOBOX_H__

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_combobox(nk_console* parent, const char* label, const char *items_separated_by_separator, int separator, int* selected);
NK_API struct nk_rect nk_console_combobox_render(nk_console* console);
NK_API void nk_console_combobox_button_click(nk_console* button);
NK_API void nk_console_combobox_button_main_click(nk_console* button);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_COMBOBOX_H__

#ifdef NK_CONSOLE_IMPLEMENTATION
#ifndef NK_CONSOLE_COMBOBOX_IMPLEMENTATION_ONCE
#define NK_CONSOLE_COMBOBOX_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Handle the click event for combobox's children items.
 */
NK_API void nk_console_combobox_button_click(nk_console* button) {
    nk_console* combobox = button->parent;

    // Find which option was selected.
    int selected = nk_console_get_widget_index(button);
    if (selected <= 0 || selected >= cvector_size(combobox->children)) {
        nk_console_button_back(button);
        return;
    }

    // Update the active selected value.
    if (combobox->combobox.selected != NULL) {
        *combobox->combobox.selected = selected - 1;
    }

    // Change the combobox text that's displayed.
    combobox->text = button->text;
    combobox->button.text_length = button->button.text_length;

    // Go back
    nk_console_button_back(button);

    // Invoke the onchange callback.
    if (combobox->onchange != NULL) {
        combobox->onchange(combobox);
    }
}

/**
 * Handle the click event for the main button for the combobox.
 *
 * @see nk_console_combobox
 * @internal
 */
NK_API void nk_console_combobox_button_main_click(nk_console* button) {
    nk_console* top = nk_console_get_top(button);
    int selected = button->combobox.selected == NULL ? 0 : *button->combobox.selected;
    if (button->children != NULL) {
        if (cvector_size(button->children) > selected + 1) {
            nk_console_set_active_widget(button->children[selected + 1]);
        }
    }

    // Switch to show all the children.
    top->activeParent = button;
}

NK_API nk_console* nk_console_combobox(nk_console* parent, const char* label, const char *items_separated_by_separator, int separator, int* selected) {
    nk_console* combobox = nk_console_label(parent, label);
    combobox->type = NK_CONSOLE_COMBOBOX;
    combobox->selectable = nk_true;
    combobox->combobox.items_separated_by_separator = items_separated_by_separator;
    combobox->combobox.separator = separator;
    combobox->combobox.selected = selected;
    combobox->combobox.label = label;
    combobox->columns = 2;
    combobox->button.symbol = NK_SYMBOL_TRIANGLE_DOWN;
    combobox->render = nk_console_combobox_render;

    // Back button
    nk_console_button_onclick(combobox, label, nk_console_combobox_button_click)
        ->button.symbol = NK_SYMBOL_TRIANGLE_UP;

    // Add all the sub-page buttons
    const char* button_text_start = items_separated_by_separator;
    int text_length = 0;
    for (int i = 0; items_separated_by_separator[i] != 0; i++) {
        text_length++;
        if (items_separated_by_separator[i] == (char)separator) {
            nk_console_button_onclick(combobox, button_text_start, nk_console_combobox_button_click)
                ->button.text_length = text_length - 1;
            text_length = 0;
            button_text_start = items_separated_by_separator + i + 1;
        }
    }

    // Add the last item
    nk_console_button_onclick(combobox, button_text_start, nk_console_combobox_button_click)
                ->button.text_length = text_length;

    if (selected != NULL) {
        if (*selected < 0) {
            *selected = 0;
        }
        else if (*selected >= cvector_size(combobox->children) - 1) {
            *selected = cvector_size(combobox->children) - 2;
        }

        combobox->text = combobox->children[*selected + 1]->text;
        combobox->button.text_length = combobox->children[*selected + 1]->button.text_length;
    }

    combobox->button.onclick = nk_console_combobox_button_main_click;

    return combobox;
}

NK_API struct nk_rect nk_console_combobox_render(nk_console* console) {
    nk_console* top = nk_console_get_top(console);
    int desired_columns = nk_strlen(console->combobox.label) > 0 ? console->columns : console->columns - 1;
    if (desired_columns > 0) {
        nk_layout_row_dynamic(console->context, 0, desired_columns);
    }

    // Allow changing the value with left/right
    if (!console->disabled && nk_console_is_active_widget(console) && !top->input_processed) {
        if (console->combobox.selected != NULL && console->children != NULL) {
            nk_bool changed = nk_false;
            if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT) && *console->combobox.selected > 0) {
                *console->combobox.selected = *console->combobox.selected - 1;
                changed = nk_true;
            }
            else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT) && *console->combobox.selected < cvector_size(console->children) - 2) {
                *console->combobox.selected = *console->combobox.selected + 1;
                changed = nk_true;
            }

            if (changed) {
                console->text = console->children[*console->combobox.selected + 1]->text;
                console->button.text_length = console->children[*console->combobox.selected + 1]->button.text_length;
                if (console->onchange != NULL) {
                    console->onchange(console);
                }
            }
        }
    }

    // Display the label
    if (nk_strlen(console->combobox.label) > 0) {
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_begin(console->context);
        }
        nk_label(console->context, console->combobox.label, NK_TEXT_LEFT);
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_end(console->context);
        }
    }

    // Display the mocked combobox button
    int swap_columns = console->columns;
    console->columns = 0;
    //console->type = NK_CONSOLE_BUTTON;
    if (nk_console_is_active_widget(console)) {
        console->button.symbol = NK_SYMBOL_TRIANGLE_DOWN;
    }
    else {
        console->button.symbol = NK_SYMBOL_NONE;
    }
    struct nk_rect widget_bounds = nk_console_button_render(console);
    console->columns = swap_columns;
    return widget_bounds;
}

#if defined(__cplusplus)
}
#endif

#endif
#endif
