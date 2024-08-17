#ifndef NK_CONSOLE_RADIO_H__
#define NK_CONSOLE_RADIO_H__

/**
 * Data for the radio button.
 *
 * @see nk_console_radio()
 */
typedef struct nk_console_radio_data {
    int* selected; // The selected index for the radio button within the radio group.
} nk_console_radio_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Creates a radio button. Use the same selected pointer to indicate the same radio buttons within the radio group.
 *
 * @param parent The parent widget.
 * @param label The label for the radio button.
 * @param selected The selected index for the radio button. Provide the same selected index for all radio buttons in the group.
 * @return The radio button widget.
 */
NK_API nk_console* nk_console_radio(nk_console* parent, const char* label, int* selected);

/**
 * Renders the radio button.
 *
 * @param widget The radio button widget.
 * @return The bounds of the radio button.
 */
NK_API struct nk_rect nk_console_radio_render(nk_console* widget);

/**
 * Returns true if the radio button is the selected radio in the group.
 *
 * @param widget The radio button widget.
 * @return True if the radio button is selected.
 */
NK_API nk_bool nk_console_radio_is_selected(nk_console* widget);

/**
 * Returns the index of the radio button in the group.
 *
 * @param widget The radio button widget.
 * @return The index of the radio button.
 */
NK_API int nk_console_radio_index(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_RADIO_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_RADIO_IMPLEMENTATION_ONCE
#define NK_CONSOLE_RADIO_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API int nk_console_radio_index(nk_console* widget) {
    if (widget == NULL || widget->data == NULL || widget->parent == NULL || widget->parent->children == NULL) {
        return -1;
    }

    nk_console_radio_data* data = (nk_console_radio_data*)widget->data;
    if (data->selected == NULL) {
        return -1;
    }

    int index = 0;
    for (size_t i = 0; i < cvector_size(widget->parent->children); i++) {
        nk_console* child = widget->parent->children[i];
        if (child->type == NK_CONSOLE_RADIO && child->data != NULL) {
            nk_console_radio_data* child_data = (nk_console_radio_data*)child->data;
            if (data->selected == child_data->selected) {
                if (widget == child) {
                    return index;
                }
                index++;
            }
        }
    }

    return -1;
}

NK_API nk_bool nk_console_radio_is_selected(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) {
        return nk_false;
    }
    int index = nk_console_radio_index(widget);
    if (index < 0) {
        return nk_false;
    }
    nk_console_radio_data* data = (nk_console_radio_data*)widget->data;
    if (data->selected == NULL) {
        return nk_false;
    }
    return *data->selected == index;
}

NK_API struct nk_rect nk_console_radio_render(nk_console* widget) {
    if (widget == NULL || widget->label == NULL || widget->data == NULL) {
        return nk_recti(0, 0, 0, 0);
    }

    nk_console* top = nk_console_get_top(widget);
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    nk_console_radio_data* data = (nk_console_radio_data*)widget->data;

    if (data->selected == NULL) {
        return nk_recti(0, 0, 0, 0);
    }

    // Set up the layout
    nk_console_layout_widget(widget);

    // Grab some initial contexts for the radio button
    int current_value = *data->selected;
    nk_bool active = nk_console_selectable(widget) && nk_console_is_active_widget(widget);
    int index = nk_console_radio_index(widget);
    nk_bool selected = nk_console_radio_is_selected(widget);

    // Allow changing the radio value.
    if (active && !selected && !top_data->input_processed) {
        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_A)) {
            *data->selected = index;
            selected = nk_true;
            top_data->input_processed = nk_true;
            nk_console_trigger_event(widget, NK_CONSOLE_EVENT_CLICKED);
        }
    }

    // Release the disabled state if needed.
    if (widget->disabled) {
        nk_widget_disable_begin(widget->ctx);
    }

    // Style
    struct nk_style_item style = widget->ctx->style.option.normal;
    if (active) {
        if (selected) {
            widget->ctx->style.option.normal = widget->ctx->style.option.active;
        }
        else {
            widget->ctx->style.option.normal = widget->ctx->style.option.hover;
        }
    }

    // Display the ratio button
    nk_bool radio_active = selected;
    if (widget->alignment != 0) {
        nk_flags text_alignment = 0;
        nk_flags widget_alignment = 0;
        switch (widget->alignment) {
            case NK_TEXT_LEFT:
                text_alignment = NK_TEXT_ALIGN_LEFT;
                widget_alignment = NK_WIDGET_ALIGN_RIGHT;
                break;
            case NK_TEXT_RIGHT:
                text_alignment = NK_TEXT_ALIGN_RIGHT;
                widget_alignment = NK_WIDGET_ALIGN_LEFT;
                break;
            case NK_TEXT_CENTERED:
                text_alignment = NK_TEXT_ALIGN_CENTERED;
                widget_alignment = NK_WIDGET_ALIGN_CENTERED;
                break;
        }

        if (widget->label_length > 0) {
            nk_radio_text_align(widget->ctx, widget->label, widget->label_length, &radio_active, widget_alignment, text_alignment);
        }
        else {
            nk_radio_label_align(widget->ctx, widget->label, &radio_active, widget_alignment, text_alignment);
        }
    }
    else {
        if (widget->label_length > 0) {
            nk_radio_text(widget->ctx, widget->label, widget->label_length, &radio_active);
        }
        else {
            nk_radio_label(widget->ctx, widget->label, &radio_active);
        }
    }

    if (radio_active == nk_true && radio_active != selected) {
        *data->selected = index;
        nk_console_trigger_event(widget, NK_CONSOLE_EVENT_CLICKED);
        top_data->input_processed = nk_true;
    }

    // Restore styles
    widget->ctx->style.option.normal = style;

    // Release the disabled state if needed.
    if (widget->disabled) {
        nk_widget_disable_end(widget->ctx);
    }

    // Trigger the value changed event
    if (current_value != *data->selected) {
        nk_console_trigger_event(widget, NK_CONSOLE_EVENT_CHANGED);
    }

    // Since labels don't really have widget bounds, we get the bounds after the label is displayed as a work-around.
    struct nk_rect widget_bounds = nk_layout_widget_bounds(widget->ctx);

    // Allow switching up/down in widgets
    if (active) {
        nk_console_check_up_down(widget, widget_bounds);
        nk_console_check_tooltip(widget);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_radio(nk_console* parent, const char* label, int* selected) {
    if (parent == NULL || selected == NULL) {
        return NULL;
    }

    nk_console_radio_data* data = (nk_console_radio_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_radio_data));
    nk_zero(data, sizeof(nk_console_radio_data));
    data->selected = selected;

    nk_console* widget = nk_console_label(parent, label);
    widget->type = NK_CONSOLE_RADIO;
    widget->selectable = nk_true;
    widget->columns = 1;
    widget->data = data;
    widget->render = nk_console_radio_render;
    widget->alignment = 0;
    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_RADIO_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
