#ifndef NK_CONSOLE_CHECKBOX_H__
#define NK_CONSOLE_CHECKBOX_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_checkbox_data {
    nk_bool* value_bool;
} nk_console_checkbox_data;

NK_API nk_console* nk_console_checkbox(nk_console* parent, const char* text, nk_bool* active);
NK_API struct nk_rect nk_console_checkbox_render(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_CHECKBOX_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_CHECKBOX_IMPLEMENTATION_ONCE
#define NK_CONSOLE_CHECKBOX_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_checkbox_render(nk_console* console) {
    nk_console_checkbox_data* data = (nk_console_checkbox_data*)console->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(console);

    struct nk_rect widget_bounds = nk_layout_widget_bounds(console->ctx);
    nk_console* top = nk_console_get_top(console);

    // Allow changing the checkbox value.
    nk_bool active = nk_false;
    if (!console->disabled && nk_console_is_active_widget(console) && !top->input_processed) {
        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_A)) {
            if (data->value_bool != NULL) {
                *data->value_bool = !*data->value_bool;
                nk_console_onchange(console);
            }
            active = nk_true;
            top->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT)) {
            if (data->value_bool != NULL) {
                *data->value_bool = nk_false;
                nk_console_onchange(console);
            }
            active = nk_true;
            top->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT)) {
            if (data->value_bool != NULL) {
                *data->value_bool = nk_true;
                nk_console_onchange(console);
            }
            active = nk_true;
            top->input_processed = nk_true;
        }
    }

    // Style
    struct nk_style_item checkboxStyle = console->ctx->style.checkbox.normal;
    if (nk_console_is_active_widget(console)) {
        if (active) {
            console->ctx->style.checkbox.normal = console->ctx->style.checkbox.active;
        }
        else {
            console->ctx->style.checkbox.normal = console->ctx->style.checkbox.hover;
        }
    }

    if (console->disabled || !nk_console_is_active_widget(console)) {
        nk_widget_disable_begin(console->ctx);
    }

    // Display the checkbox with fixed alignment.
    nk_bool changed = nk_false;
    if (console->alignment == NK_TEXT_LEFT) {
        changed = nk_checkbox_label_align(console->ctx, console->label, data->value_bool, NK_TEXT_RIGHT, NK_TEXT_LEFT);
    }
    else {
        changed = nk_checkbox_label(console->ctx, console->label, data->value_bool);
    }

    // Invoke onchanged event.
    if (changed) {
        nk_console_onchange(console);
    }

    if (console->disabled || !nk_console_is_active_widget(console)) {
        nk_widget_disable_end(console->ctx);
    }

    // Restore the styles
    console->ctx->style.checkbox.normal = checkboxStyle;

    // Allow switching up/down in widgets
    if (nk_console_is_active_widget(console)) {
        nk_console_check_up_down(console, widget_bounds);
        nk_console_check_tooltip(console);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_checkbox(nk_console* parent, const char* text, nk_bool* active) {
    NK_ASSERT(active != NULL);
    nk_console_checkbox_data* data = (nk_console_checkbox_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_checkbox_data));
    nk_zero(data, sizeof(nk_console_checkbox_data));

    nk_console* checkbox = nk_console_label(parent, text);
    checkbox->render = nk_console_checkbox_render;
    data->value_bool = active;
    checkbox->type = NK_CONSOLE_CHECKBOX;
    checkbox->selectable = nk_true;
    checkbox->columns = 1;
    checkbox->data = (void*)data;
    return checkbox;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_CHECKBOX_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
