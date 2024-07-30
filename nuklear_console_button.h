#ifndef NK_CONSOLE_BUTTON_H__
#define NK_CONSOLE_BUTTON_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_button_data {
    enum nk_symbol_type symbol;
    nk_console_event_handler onclick;
    struct nk_image image;
} nk_console_button_data;

NK_API nk_console* nk_console_button(nk_console* parent, const char* text);
NK_API struct nk_rect nk_console_button_render(nk_console* console);
NK_API void nk_console_button_back(nk_console* button);
NK_API nk_console* nk_console_button_onclick(nk_console* parent, const char* text, nk_console_event onclick);
NK_API nk_console* nk_console_button_onclick_handler(nk_console* parent, const char* text, nk_console_event_handler onclick);

NK_API enum nk_symbol_type nk_console_button_get_symbol(nk_console* button);
NK_API void nk_console_button_set_symbol(nk_console* button, enum nk_symbol_type symbol);
NK_API nk_console_event nk_console_button_get_onclick(nk_console* button);
NK_API void nk_console_button_set_onclick(nk_console* button, nk_console_event onclick);
NK_API void nk_console_button_set_onclick_handler(nk_console* button, nk_console_event_handler onclick);

NK_API void nk_console_button_set_image(nk_console* button, struct nk_image image);
NK_API struct nk_image nk_console_button_get_image(nk_console* button);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_BUTTON_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_BUTTON_IMPLEMENTATION_ONCE
#define NK_CONSOLE_BUTTON_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API enum nk_symbol_type nk_console_button_get_symbol(nk_console* button) {
    if (button == NULL || button->data == NULL) {
        return NK_SYMBOL_NONE;
    }
    nk_console_button_data* data = (nk_console_button_data*)button->data;
    return data->symbol;
}

NK_API void nk_console_button_set_symbol(nk_console* button, enum nk_symbol_type symbol) {
    if (button == NULL || button->data == NULL) {
        return;
    }
    nk_console_button_data* data = (nk_console_button_data*)button->data;
    data->symbol = symbol;
}

NK_API nk_console_event nk_console_button_get_onclick(nk_console* button) {
    if (button == NULL || button->data == NULL) {
        return NULL;
    }
    nk_console_button_data* data = (nk_console_button_data*)button->data;
    return data->onclick.data.event;
}

NK_API void nk_console_button_set_onclick(nk_console* button, nk_console_event onclick) {
    nk_console_event_handler handler = {0};
    if (onclick) {
        handler.callback = &nk_console_call_event;
        handler.data.event = onclick;
    }
    nk_console_button_set_onclick_handler(button, handler);
}

NK_API void nk_console_button_set_onclick_handler(nk_console* button, nk_console_event_handler onclick) {
    if (button == NULL || button->data == NULL) {
        return;
    }

    nk_console_button_data* data = (nk_console_button_data*)button->data;
    nk_console_event_handler_destroy(&data->onclick, button);

    data->onclick = onclick;
}

NK_API void nk_console_button_set_image(nk_console* button, struct nk_image image) {
    if (button == NULL || button->data == NULL) {
        return;
    }
    nk_console_button_data* data = (nk_console_button_data*)button->data;
    data->image = image;

    // While automatically setting the height to the button height is an option here, we will opt out of doing that.
    //button->height = (int)image.h;
}

NK_API struct nk_image nk_console_button_get_image(nk_console* button) {
    if (button == NULL || button->data == NULL) {
        struct nk_image output = {0};
        return output;
    }
    nk_console_button_data* data = (nk_console_button_data*)button->data;
    return data->image;
}

NK_API struct nk_rect nk_console_button_render(nk_console* console) {
    nk_console_button_data* data = (nk_console_button_data*)console->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console* top = nk_console_get_top(console);

    nk_console_layout_widget(console);

    struct nk_rect widget_bounds = nk_layout_widget_bounds(console->ctx);

    if (console->disabled) {
        nk_widget_disable_begin(console->ctx);
    }

    // Check the button state.
    nk_bool selected = nk_false;
    if (!console->disabled && nk_console_is_active_widget(console) && !top->input_processed && nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_A)) {
        selected = nk_true;
    }

    // Apply the style.
    struct nk_style_item buttonStyle = console->ctx->style.button.normal;
    if (nk_console_is_active_widget(console)) {
        if (selected) {
            console->ctx->style.button.normal = console->ctx->style.button.active;
        }
        else {
            console->ctx->style.button.normal = console->ctx->style.button.hover;
        }
    }

    // Display the button.
    if (data->image.region[3] == 0) {
        // No image
        if (console->label_length <= 0) {
            // Check if there is a Label
            if (console->label != NULL && nk_strlen(console->label) > 0) {
                if (data->symbol == NK_SYMBOL_NONE) {
                    selected |= nk_button_label(console->ctx, console->label);
                }
                else {
                    selected |= nk_button_symbol_label(console->ctx, data->symbol, console->label, console->alignment);
                }
            }
            else {
                // Display the button as just a symbol?
                selected |= nk_button_symbol(console->ctx, data->symbol);
            }
        }
        else {
            if (data->symbol == NK_SYMBOL_NONE) {
                selected |= nk_button_text(console->ctx, console->label, console->label_length);
            }
            else {
                selected |= nk_button_symbol_text(console->ctx, data->symbol, console->label, console->label_length, console->alignment);
            }
        }
    }
    else {
        // Display the button with an image
        if (console->label_length > 0) {
            selected |= nk_button_image_text(console->ctx, data->image, console->label, console->label_length, console->alignment);
        }
        else if (console->label != NULL && nk_strlen(console->label) > 0) {
            selected |= nk_button_image_label(console->ctx, data->image, console->label, console->alignment);
        }
        else {
            selected |= nk_button_image(console->ctx, data->image);
        }
    }

    // Restore the styles
    console->ctx->style.button.normal = buttonStyle;

    // Act on the button
    if (selected) {
        top->input_processed = nk_true;

        // If there's no onclick action and there are children...
        if (data->onclick.callback == NULL) {
            if (console->children != NULL) {
                //top->activeParent = console;
                nk_console_set_active_parent(console);
            }
        }
        else {
            data->onclick.callback(data->onclick.data, console);
        }
    }

    if (console->disabled) {
        nk_widget_disable_end(console->ctx);
    }

    // Allow switching up/down in widgets
    if (nk_console_is_active_widget(console)) {
        nk_console_check_up_down(console, widget_bounds);
        nk_console_check_tooltip(console);
    }

    return widget_bounds;
}

/**
 * Create a button.
 */
NK_API nk_console* nk_console_button(nk_console* parent, const char* text) {
    return nk_console_button_onclick(parent, text, NULL);
}

/**
 * Take action on a BACK button.
 */
NK_API void nk_console_button_back(nk_console* button) {
    if (button == NULL) {
        return;
    }

    nk_console* top = nk_console_get_top(button);
    if (top == NULL) {
        return;
    }

    nk_console* parent = button->parent;
    if (parent != NULL) {
        parent = parent->parent;
    }
    if (parent != NULL) {
        nk_console_set_active_parent(parent);
    }
    else {
        top->activeParent = NULL;
    }
}

static void nk_console_button_destroy(nk_console* button) {
    nk_console_button_data* data = (nk_console_button_data*)button->data;
    if (data == NULL) {
        return;
    }
    nk_console_event_handler_destroy(&data->onclick, button);
}

NK_API nk_console* nk_console_button_onclick(nk_console* parent, const char* text, nk_console_event onclick) {
    nk_console_event_handler handler = {0};
    if (onclick) {
        handler.callback = &nk_console_call_event;
        handler.data.event = onclick;
    }
    return nk_console_button_onclick_handler(parent, text, handler);
}

NK_API nk_console* nk_console_button_onclick_handler(nk_console* parent, const char* text, nk_console_event_handler onclick) {
    // Create the widget data.
    nk_console_button_data* data = (nk_console_button_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_button_data));
    nk_zero(data, sizeof(nk_console_button_data));

    nk_console* button = nk_console_label(parent, text);
    button->type = NK_CONSOLE_BUTTON;
    button->data = (void*)data;
    button->selectable = nk_true;
    button->columns = 1;
    button->render = nk_console_button_render;
    button->destroy = &nk_console_button_destroy;
    nk_console_button_set_onclick_handler(button, onclick);
    return button;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_BUTTON_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
