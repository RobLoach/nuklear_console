#ifndef NK_CONSOLE_COLOR_H__
#define NK_CONSOLE_COLOR_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_color_data {
    struct nk_console_button_data button; /** Inherited from button */
    struct nk_colorf* color; /** The color which will be manipulated by the user. */
} nk_console_color_data;

/**
 * Create a color picker widget.
 *
 * @param parent The parent widget.
 * @param label The label for the widget
 * @param color A pointer to where the color should be held.
 * @param format Whether the color should be in RGB or RGBA format.
 *
 * @return The created widget.
 */
NK_API nk_console* nk_console_color(nk_console* parent, const char* label, struct nk_colorf* color, enum nk_color_format format);

/**
 * Render callback to display a color widget.
 */
NK_API struct nk_rect nk_console_color_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_COLOR_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_COLOR_IMPLEMENTATION_ONCE
#define NK_CONSOLE_COLOR_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_color_render(nk_console* console) {
    if (console == NULL || console->data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }
    nk_console_color_data* data = (nk_console_color_data*)console->data;
    if (data->color == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(console);

    // Display the label
    if (console->label != NULL && console->label[0] != '\0') {
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_begin(console->ctx);
        }
        if (console->label_length > 0) {
            nk_text(console->ctx, console->label, console->label_length, NK_TEXT_LEFT);
        }
        else {
            nk_label(console->ctx, console->label, NK_TEXT_LEFT);
        }
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_end(console->ctx);
        }
    }

    // Swap the colors for the button
    struct nk_color current_color = nk_rgb_cf(*data->color);
    struct nk_color swap_text_normal = console->ctx->style.button.text_normal;
    struct nk_color swap_text_active = console->ctx->style.button.text_active;
    struct nk_color swap_text_hover = console->ctx->style.button.text_hover;
    struct nk_color swap_text_background = console->ctx->style.button.text_background;
    console->ctx->style.button.text_normal = current_color;
    console->ctx->style.button.text_active = current_color;
    console->ctx->style.button.text_hover = current_color;
    console->ctx->style.button.text_background = current_color;

    // Display the mocked button
    int swap_columns = console->columns;
    const char* swap_label = console->label;
    int swap_label_length = console->label_length;
    console->columns = 0;
    console->label = NULL;
    console->label_length = 0;
    struct nk_rect widget_bounds = nk_console_button_render(console);
    console->columns = swap_columns;
    console->label = swap_label;
    console->label_length = swap_label_length;

    console->ctx->style.button.text_normal = swap_text_normal;
    console->ctx->style.button.text_active = swap_text_active;
    console->ctx->style.button.text_hover = swap_text_hover;
    console->ctx->style.button.text_background = swap_text_background;

    return widget_bounds;
}

static void nk_console_color_event_changed(nk_console* property, void* user_data) {
    if (property == NULL || property->parent == NULL) {
        return;
    }
    NK_UNUSED(user_data);
    nk_console_trigger_event(property->parent, NK_CONSOLE_EVENT_CHANGED);
}

NK_API nk_console* nk_console_color(nk_console* parent, const char* label, struct nk_colorf* color, enum nk_color_format format) {
    if (parent == NULL || color == NULL) {
        return NULL;
    }

    // Create the widget data.
    nk_console_color_data* data = (nk_console_color_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_color_data));
    nk_zero(data, sizeof(nk_console_color_data));
    data->color = color;

    nk_console* widget = nk_console_label(parent, label);
    widget->type = NK_CONSOLE_COLOR;
    widget->columns = label == NULL ? 1 : 2;
    widget->render = nk_console_color_render;
    widget->selectable = nk_true;
    widget->data = data;
    nk_console_button_set_symbol(widget, NK_SYMBOL_RECT_SOLID);

    // Active Color.
    if (label != NULL && nk_strlen(label) != 1 && label[0] != '@') {
        nk_console* color_display = nk_console_color(widget, "@", color, format);
        nk_console_add_event(color_display, NK_CONSOLE_EVENT_CLICKED, &nk_console_button_back);
        color_display->label = NULL;
        color_display->columns = 1;

        // Don't need any of the children.
        nk_console_free_children(color_display);
    }

    // Add the color components
    nk_console_add_event(nk_console_slider_float(widget, "Red", 0.0f, &color->r, 1.0f, 0.05f), NK_CONSOLE_EVENT_CHANGED, &nk_console_color_event_changed);
    nk_console_add_event(nk_console_slider_float(widget, "Green", 0.0f, &color->g, 1.0f, 0.05f), NK_CONSOLE_EVENT_CHANGED, &nk_console_color_event_changed);
    nk_console_add_event(nk_console_slider_float(widget, "Blue", 0.0f, &color->b, 1.0f, 0.05f), NK_CONSOLE_EVENT_CHANGED, &nk_console_color_event_changed);
    if (format == NK_RGBA) {
        nk_console_add_event(nk_console_slider_float(widget, "Alpha", 0.0f, &color->a, 1.0f, 0.05f), NK_CONSOLE_EVENT_CHANGED, &nk_console_color_event_changed);
    }

    // Back Button
    nk_console_button_onclick(widget, "Back", &nk_console_button_back);

    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_COLOR_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
