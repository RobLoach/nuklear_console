#ifndef NK_CONSOLE_PROPERTY_H__
#define NK_CONSOLE_PROPERTY_H__

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_property_int(nk_console* parent, const char* label, int min, int *val, int max, int step, float inc_per_pixel);
NK_API nk_console* nk_console_property_float(nk_console* parent, const char* label, float min, float *val, float max, float step, float inc_per_pixel);
NK_API nk_console* nk_console_slider_int(nk_console* parent, const char* label, int min, int* val, int max, int step);
NK_API nk_console* nk_console_slider_float(nk_console* parent, const char* label, float min, float* val, float max, float step);
NK_API struct nk_rect nk_console_property_render(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_PROPERTY_H__

#ifdef NK_CONSOLE_IMPLEMENTATION
#ifndef NK_CONSOLE_PROPERTY_IMPLEMENTATION_ONCE
#define NK_CONSOLE_PROPERTY_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_property_render(nk_console* console) {
    if (console->columns > 0) {
        nk_layout_row_dynamic(console->context, 0, console->columns);
    }
    nk_console* top = nk_console_get_top(console);

    // Allow changing the value with left/right
    if (!console->disabled && nk_console_is_active_widget(console) && !top->input_processed) {
        if (nk_input_is_key_pressed(&console->context->input, NK_KEY_LEFT)) {
            switch (console->type) {
                case NK_CONSOLE_SLIDER_INT:
                case NK_CONSOLE_PROPERTY_INT:
                    *console->property.val_int = *console->property.val_int - console->property.step_int;
                    if (*console->property.val_int < console->property.min_int) {
                        *console->property.val_int = console->property.min_int;
                    }
                    break;
                case NK_CONSOLE_SLIDER_FLOAT:
                case NK_CONSOLE_PROPERTY_FLOAT:
                    *console->property.val_float = *console->property.val_float - console->property.step_float;
                    if (*console->property.val_float < console->property.min_float) {
                        *console->property.val_float = console->property.min_float;
                    }
                    break;
            }
            if (console->onchange != NULL) {
                console->onchange(console);
            }
            top->input_processed = nk_true;
        }
        else if (nk_input_is_key_pressed(&console->context->input, NK_KEY_RIGHT)) {
            switch (console->type) {
                case NK_CONSOLE_SLIDER_INT:
                case NK_CONSOLE_PROPERTY_INT:
                    *console->property.val_int = *console->property.val_int + console->property.step_int;
                    if (*console->property.val_int > console->property.max_int) {
                        *console->property.val_int = console->property.max_int;
                    }
                    break;
                case NK_CONSOLE_SLIDER_FLOAT:
                case NK_CONSOLE_PROPERTY_FLOAT:
                    *console->property.val_float = *console->property.val_float + console->property.step_float;
                    if (*console->property.val_float > console->property.max_float) {
                        *console->property.val_float = console->property.max_float;
                    }
                    break;
            }
            if (console->onchange != NULL) {
                console->onchange(console);
            }
            top->input_processed = nk_true;
        }
    }

    // Style
    enum nk_symbol_type left = console->context->style.property.sym_left;
    enum nk_symbol_type right = console->context->style.property.sym_right;
    struct nk_color bar_normal = console->context->style.slider.bar_normal;
    struct nk_style_item cursor_normal = console->context->style.slider.cursor_normal;

    if (!nk_console_is_active_widget(console)) {
        console->context->style.property.sym_left = NK_SYMBOL_NONE;
        console->context->style.property.sym_right = NK_SYMBOL_NONE;
    }
    else {
        console->context->style.slider.bar_normal = console->context->style.slider.bar_hover;
        console->context->style.slider.cursor_normal = console->context->style.slider.cursor_hover;
    }

    // Display the label
    if (!nk_console_is_active_widget(console)) {
        nk_widget_disable_begin(console->context);
    }
    nk_label(console->context, console->text, NK_TEXT_LEFT);
    if (!nk_console_is_active_widget(console)) {
        nk_widget_disable_end(console->context);
    }

    struct nk_rect widget_bounds = nk_layout_widget_bounds(console->context);

    if (console->disabled) {
        nk_widget_disable_begin(console->context);
    }

    // Display the widget
    switch (console->type) {
        case NK_CONSOLE_PROPERTY_INT:
            nk_property_int(console->context, "", console->property.min_int, console->property.val_int, console->property.max_int, console->property.step_int, console->property.inc_per_pixel);
            break;
        case NK_CONSOLE_PROPERTY_FLOAT:
            nk_property_float(console->context, "", console->property.min_float, console->property.val_float, console->property.max_float, console->property.step_float, console->property.inc_per_pixel);
            break;
        case NK_CONSOLE_SLIDER_INT:
            nk_slider_int(console->context, console->property.min_int, console->property.val_int, console->property.max_int, console->property.step_int);
            break;
        case NK_CONSOLE_SLIDER_FLOAT:
            nk_slider_float(console->context, console->property.min_float, console->property.val_float, console->property.max_float, console->property.step_float);
            break;
    }

    // Style Restoration
    if (!nk_console_is_active_widget(console)) {
        console->context->style.property.sym_left = left;
        console->context->style.property.sym_right = right;
    }
    else {
        console->context->style.slider.bar_normal = bar_normal;
        console->context->style.slider.cursor_normal = cursor_normal;
    }

    if (console->disabled) {
        nk_widget_disable_end(console->context);
    }

    // Allow switching up/down in widgets
    if (nk_console_is_active_widget(console)) {
        nk_console_check_up_down(console, widget_bounds);
        nk_console_tooltip(console, widget_bounds);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_property_int(nk_console* parent, const char* label, int min, int *val, int max, int step, float inc_per_pixel) {
    NK_ASSERT(val);
    nk_console* widget = nk_console_label(parent, label);
    widget->render = nk_console_property_render;
    widget->type = NK_CONSOLE_PROPERTY_INT;
    widget->selectable = nk_true;
    widget->property.min_int = min;
    widget->property.val_int = val;
    widget->property.max_int = max;
    widget->property.step_int = step;
    widget->property.inc_per_pixel = inc_per_pixel;
    widget->columns = 2;
    if (*val < min) {
        *val = min;
    }
    else if (*val > max) {
        *val = max;
    }
    return widget;
}

NK_API nk_console* nk_console_property_float(nk_console* parent, const char* label, float min, float *val, float max, float step, float inc_per_pixel) {
    NK_ASSERT(val);
    nk_console* widget = nk_console_label(parent, label);
    widget->render = nk_console_property_render;
    widget->type = NK_CONSOLE_PROPERTY_FLOAT;
    widget->selectable = nk_true;
    widget->property.min_float = min;
    widget->property.val_float = val;
    widget->property.max_float = max;
    widget->property.step_float = step;
    widget->property.inc_per_pixel = inc_per_pixel;
    widget->columns = 2;
    if (*val < min) {
        *val = min;
    }
    else if (*val > max) {
        *val = max;
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

#endif
#endif
