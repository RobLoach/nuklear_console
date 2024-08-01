#ifndef NK_CONSOLE_PROPERTY_H__
#define NK_CONSOLE_PROPERTY_H__

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Data for Property and Slider widgets.
 */
typedef struct nk_console_property_data {
    int min_int; /** The minimum value, represented as an integer. */
    int max_int; /** The maximum value, represented as an integer. */
    int step_int; /** How much each step should increment. */
    float min_float; /** The minimum value, represented as a float. */
    float max_float; /** The maximum value, represented as a float. */
    float step_float; /** How much each step should increment. */
    float inc_per_pixel; /** The increment per pixel value as a float. */
    int* val_int; /** Pointer to the integer value. */
    float* val_float; /** Pointer to the float value. */
} nk_console_property_data;

NK_API nk_console* nk_console_property_int(nk_console* parent, const char* label, int min, int *val, int max, int step, float inc_per_pixel);
NK_API nk_console* nk_console_property_float(nk_console* parent, const char* label, float min, float *val, float max, float step, float inc_per_pixel);
NK_API nk_console* nk_console_slider_int(nk_console* parent, const char* label, int min, int* val, int max, int step);
NK_API nk_console* nk_console_slider_float(nk_console* parent, const char* label, float min, float* val, float max, float step);
NK_API struct nk_rect nk_console_property_render(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_PROPERTY_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_PROPERTY_IMPLEMENTATION_ONCE
#define NK_CONSOLE_PROPERTY_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_property_render(nk_console* console) {
    nk_console_property_data* data = (nk_console_property_data*)console->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    if (console->type == NK_CONSOLE_PROPERTY_INT || console->type == NK_CONSOLE_SLIDER_INT) {
        if (data->val_int == NULL) {
            return nk_rect(0, 0, 0, 0);
        }
    }
    else if (console->type == NK_CONSOLE_PROPERTY_FLOAT || console->type == NK_CONSOLE_SLIDER_FLOAT) {
        if (data->val_float == NULL) {
            return nk_rect(0, 0, 0, 0);
        }
    }

    nk_console_layout_widget(console);

    nk_console* top = nk_console_get_top(console);

    // Allow changing the value with left/right
    if (!console->disabled && nk_console_is_active_widget(console) && !top->input_processed) {
        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT)) {
            switch (console->type) {
                case NK_CONSOLE_SLIDER_INT:
                case NK_CONSOLE_PROPERTY_INT:
                    *data->val_int = *data->val_int - data->step_int;
                    if (*data->val_int < data->min_int) {
                        *data->val_int = data->min_int;
                    }
                    break;
                case NK_CONSOLE_SLIDER_FLOAT:
                case NK_CONSOLE_PROPERTY_FLOAT:
                    *data->val_float = *data->val_float - data->step_float;
                    if (*data->val_float < data->min_float) {
                        *data->val_float = data->min_float;
                    }
                    break;
                default:
                    // Nothing.
                    break;
            }
            nk_console_onchange(console);
            top->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT)) {
            switch (console->type) {
                case NK_CONSOLE_SLIDER_INT:
                case NK_CONSOLE_PROPERTY_INT:
                    *data->val_int = *data->val_int + data->step_int;
                    if (*data->val_int > data->max_int) {
                        *data->val_int = data->max_int;
                    }
                    break;
                case NK_CONSOLE_SLIDER_FLOAT:
                case NK_CONSOLE_PROPERTY_FLOAT:
                    *data->val_float = *data->val_float + data->step_float;
                    if (*data->val_float > data->max_float) {
                        *data->val_float = data->max_float;
                    }
                    break;
                default:
                    // Nothing
                    break;
            }
            nk_console_onchange(console);
            top->input_processed = nk_true;
        }
    }

    // Style
    enum nk_symbol_type left = console->ctx->style.property.sym_left;
    enum nk_symbol_type right = console->ctx->style.property.sym_right;
    struct nk_color bar_normal = console->ctx->style.slider.bar_normal;
    struct nk_style_item cursor_normal = console->ctx->style.slider.cursor_normal;

    if (!nk_console_is_active_widget(console)) {
        console->ctx->style.property.sym_left = NK_SYMBOL_NONE;
        console->ctx->style.property.sym_right = NK_SYMBOL_NONE;
    }
    else {
        console->ctx->style.slider.bar_normal = console->ctx->style.slider.bar_hover;
        console->ctx->style.slider.cursor_normal = console->ctx->style.slider.cursor_hover;
    }

    // Display the label
    if (nk_strlen(console->label) > 0) {
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_begin(console->ctx);
        }
        nk_label(console->ctx, console->label, NK_TEXT_LEFT);
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_end(console->ctx);
        }
    }

    struct nk_rect widget_bounds = nk_layout_widget_bounds(console->ctx);

    if (console->disabled) {
        nk_widget_disable_begin(console->ctx);
    }

    // Display the widget
    char name[NK_MAX_NUMBER_BUFFER];
    NK_MEMCPY(name + 2, console->label, nk_strlen(console->label) + 1);
    name[0] = '#';
    name[1] = '#';
    switch (console->type) {
        case NK_CONSOLE_PROPERTY_INT:
            nk_property_int(console->ctx, name, data->min_int, data->val_int, data->max_int, data->step_int, data->inc_per_pixel);
            break;
        case NK_CONSOLE_PROPERTY_FLOAT:
            nk_property_float(console->ctx, name, data->min_float, data->val_float, data->max_float, data->step_float, data->inc_per_pixel);
            break;
        case NK_CONSOLE_SLIDER_INT:
            nk_slider_int(console->ctx, data->min_int, data->val_int, data->max_int, data->step_int);
            break;
        case NK_CONSOLE_SLIDER_FLOAT:
            nk_slider_float(console->ctx, data->min_float, data->val_float, data->max_float, data->step_float);
            break;
        default:
            // Nothing
            break;
    }

    // Style Restoration
    if (!nk_console_is_active_widget(console)) {
        console->ctx->style.property.sym_left = left;
        console->ctx->style.property.sym_right = right;
    }
    else {
        console->ctx->style.slider.bar_normal = bar_normal;
        console->ctx->style.slider.cursor_normal = cursor_normal;
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

NK_API nk_console* nk_console_property_int(nk_console* parent, const char* label, int min, int *val, int max, int step, float inc_per_pixel) {
    // Create the property data.
    nk_console_property_data* data = (nk_console_property_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_property_data));
    nk_zero(data, sizeof(nk_console_property_data));
    data->min_int = min;
    data->val_int = val;
    data->max_int = max;
    data->step_int = step;
    data->inc_per_pixel = inc_per_pixel;

    nk_console* widget = nk_console_label(parent, label);
    widget->render = nk_console_property_render;
    widget->type = NK_CONSOLE_PROPERTY_INT;
    widget->selectable = nk_true;
    widget->data = (void*)data;
    widget->columns = label != NULL ? 2 : 1;

    if (val != NULL) {
        if (*val < min) {
            *val = min;
        }
        else if (*val > max) {
            *val = max;
        }
    }

    return widget;
}

NK_API nk_console* nk_console_property_float(nk_console* parent, const char* label, float min, float *val, float max, float step, float inc_per_pixel) {
    nk_console* widget = nk_console_property_int(parent, label, 0, NULL, 0, 0, inc_per_pixel);
    nk_console_property_data* data = (nk_console_property_data*)widget->data;
    widget->type = NK_CONSOLE_PROPERTY_FLOAT;
    data->min_float = min;
    data->val_float = val;
    data->max_float = max;
    data->step_float = step;

    if (val != NULL) {
        if (*val < min) {
            *val = min;
        }
        else if (*val > max) {
            *val = max;
        }
    }
    return widget;
}

NK_API nk_console* nk_console_slider_int(nk_console* parent, const char* label, int min, int* val, int max, int step) {
    nk_console* widget = nk_console_property_int(parent, label, min, val, max, step, 0);
    widget->type = NK_CONSOLE_SLIDER_INT;
    return widget;
}

NK_API nk_console* nk_console_slider_float(nk_console* parent, const char* label, float min, float* val, float max, float step) {
    nk_console* widget = nk_console_property_float(parent, label, min, val, max, step, 0);
    widget->type = NK_CONSOLE_SLIDER_FLOAT;
    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_PROPERTY_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
