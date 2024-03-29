#ifndef NK_CONSOLE_CHECKBOX_H__
#define NK_CONSOLE_CHECKBOX_H__

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_checkbox(nk_console* parent, const char* text, nk_bool* active);
NK_API struct nk_rect nk_console_checkbox_render(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_CHECKBOX_H__

#ifdef NK_CONSOLE_IMPLEMENTATION
#ifndef NK_CONSOLE_CHECKBOX_IMPLEMENTATION_ONCE
#define NK_CONSOLE_CHECKBOX_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_checkbox_render(nk_console* console) {
    if (console->columns > 0) {
        nk_layout_row_dynamic(console->context, 0, console->columns);
    }
    struct nk_rect widget_bounds = nk_layout_widget_bounds(console->context);
    nk_console* top = nk_console_get_top(console);

    // Allow changing the checkbox value.
    nk_bool active = nk_false;
    if (!console->disabled && nk_console_is_active_widget(console) && !top->input_processed) {
        if (nk_input_is_key_pressed(&console->context->input, NK_KEY_ENTER)) {
            if (console->checkbox.value_bool != NULL) {
                *console->checkbox.value_bool = !*console->checkbox.value_bool;
                if (console->onchange != NULL) {
                    console->onchange(console);
                }
            }
            active = nk_true;
            top->input_processed = nk_true;
        }
        else if (nk_input_is_key_pressed(&console->context->input, NK_KEY_LEFT)) {
            if (console->checkbox.value_bool != NULL) {
                *console->checkbox.value_bool = nk_false;
                if (console->onchange != NULL) {
                    console->onchange(console);
                }
            }
            active = nk_true;
            top->input_processed = nk_true;
        }
        else if (nk_input_is_key_pressed(&console->context->input, NK_KEY_RIGHT)) {
            if (console->checkbox.value_bool != NULL) {
                *console->checkbox.value_bool = nk_true;
                if (console->onchange != NULL) {
                    console->onchange(console);
                }
            }
            active = nk_true;
            top->input_processed = nk_true;
        }
    }

    // Style
    struct nk_style_item checkboxStyle = console->context->style.checkbox.normal;
    if (nk_console_is_active_widget(console)) {
        if (active) {
            console->context->style.checkbox.normal = console->context->style.checkbox.active;
        }
        else {
            console->context->style.checkbox.normal = console->context->style.checkbox.hover;
        }
    }

    if (console->disabled || !nk_console_is_active_widget(console)) {
        nk_widget_disable_begin(console->context);
    }

    // Display the checkbox with fixed alignment.
    nk_bool changed = nk_false;
    if (console->alignment == NK_TEXT_LEFT) {
        changed = nk_checkbox_label_align(console->context, console->text, console->checkbox.value_bool, NK_TEXT_RIGHT, NK_TEXT_LEFT);
    }
    else {
        changed = nk_checkbox_label(console->context, console->text, console->checkbox.value_bool);
    }

    // Invoke onchanged event.
    if (changed && console->onchange != NULL) {
        console->onchange(console);
    }

    if (console->disabled || !nk_console_is_active_widget(console)) {
        nk_widget_disable_end(console->context);
    }

    // Restore the styles
    console->context->style.checkbox.normal = checkboxStyle;

    // Allow switching up/down in widgets
    if (nk_console_is_active_widget(console)) {
        nk_console_check_up_down(console, widget_bounds);
        nk_console_tooltip(console, widget_bounds);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_checkbox(nk_console* parent, const char* text, nk_bool* active) {
    nk_console* checkbox = nk_console_label(parent, text);
    checkbox->render = nk_console_checkbox_render;
    checkbox->checkbox.value_bool = active;
    checkbox->type = NK_CONSOLE_CHECKBOX;
    checkbox->selectable = nk_true;
    return checkbox;
}

#if defined(__cplusplus)
}
#endif

#endif
#endif
