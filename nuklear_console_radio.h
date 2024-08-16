#ifndef NK_CONSOLE_RADIO_H__
#define NK_CONSOLE_RADIO_H__

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Data for the radio button.
 *
 * @see nk_console_radio()
 */
typedef struct nk_console_radio_data {
    int* selected;
} nk_console_radio_data;

NK_API nk_console* nk_console_radio(nk_console* parent, const char* label, int* selected);
NK_API struct nk_rect nk_console_radio_render(nk_console* widget);
NK_API nk_bool nk_console_radio_is_selected(nk_console* widget); // Returns true if the radio button is selected.
NK_API int nk_console_radio_index(nk_console* widget); // Returns the index of the radio button in the group.

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_RADIO_H__

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
    for (int i = 0; i < cvector_size(widget->parent->children); i++) {
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
    nk_console_layout_widget(widget);

    nk_bool active = nk_console_selectable(widget) && nk_console_is_active_widget(widget);
    int index = nk_console_radio_index(widget);

    // Find if it's the active selected raio.
    nk_bool selected = nk_console_radio_is_selected(widget);

    // Allow changing the radio value.
    if (active && !selected && !top_data->input_processed) {
        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_A)) {
            *data->selected = index;
            selected = nk_true;
            top_data->input_processed = nk_true;
        }
    }

    // Release the disabled state if needed.
    if (!active) {
        nk_widget_disable_end(widget->ctx);
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
    nk_radio_label(widget->ctx, widget->label, &radio_active);
    if (radio_active == nk_true && radio_active != selected) {
        *data->selected = index;
        top_data->input_processed = nk_true;
    }

    // Release the disabled state if needed.
    if (!active) {
        nk_widget_disable_end(widget->ctx);
    }

    widget->ctx->style.option.normal = style;

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
    widget->columns = label != NULL ? 2 : 1;
    widget->data = data;
    widget->render = nk_console_radio_render;
    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_RADIO_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
