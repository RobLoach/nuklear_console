#ifndef NK_CONSOLE_PROGRESS_H__
#define NK_CONSOLE_PROGRESS_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_progress_data {
    nk_size max_size;
    nk_size* value_size;
} nk_console_progress_data;

NK_API nk_console* nk_console_progress(nk_console* parent, const char* text, nk_size* current, nk_size max);
NK_API struct nk_rect nk_console_progress_render(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_PROGRESS_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_PROGRESS_IMPLEMENTATION_ONCE
#define NK_CONSOLE_PROGRESS_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_progress(nk_console* parent, const char* text, nk_size* current, nk_size max) {
    NK_ASSERT(current != NULL);
    NK_ASSERT(max > 0);

    // Create the widget data.
    nk_handle unused = {0};
    nk_console_progress_data* data = (nk_console_progress_data*)NK_CONSOLE_MALLOC(unused, NULL, sizeof(nk_console_progress_data));
    nk_zero(data, sizeof(nk_console_progress_data));

    nk_console* progress = nk_console_label(parent, text);
    progress->render = nk_console_progress_render;
    progress->type = NK_CONSOLE_PROGRESS;
    progress->selectable = nk_true;
    data->value_size = current;
    data->max_size = max;
    progress->columns = text != NULL ? 2 : 1;
    progress->data = (void*)data;
    if (*current > max) {
        *current = max;
    }
    return progress;
}

NK_API struct nk_rect nk_console_progress_render(nk_console* console) {
    nk_console_progress_data* data = (nk_console_progress_data*)console->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(console);

    nk_console* top = nk_console_get_top(console);

    // Allow changing the value.
    nk_bool active = nk_false;
    if (!console->disabled && nk_console_is_active_widget(console) && !top->input_processed) {
        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT)) {
            if (data->value_size != NULL && *data->value_size > 0) {
                *data->value_size = *data->value_size - 1;
                if (console->onchange != NULL) {
                    console->onchange(console);
                }
            }
            active = nk_true;
            top->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT)) {
            if (data->value_size != NULL && *data->value_size < data->max_size) {
                *data->value_size = *data->value_size + 1;
                if (console->onchange != NULL) {
                    console->onchange(console);
                }
            }
            active = nk_true;
            top->input_processed = nk_true;
        }
    }

    // Display the label
    if (nk_strlen(console->label) > 0) {
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_begin(console->context);
        }
        nk_label(console->context, console->label, NK_TEXT_LEFT);
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_end(console->context);
        }
    }

    struct nk_rect widget_bounds = nk_layout_widget_bounds(console->context);

    if (console->disabled) {
        nk_widget_disable_begin(console->context);
    }

    // Progress
    struct nk_style_item cursor_normal = console->context->style.progress.cursor_normal;
    struct nk_style_item cursor_hover = console->context->style.progress.cursor_hover;
    struct nk_style_item cursor_active = console->context->style.progress.cursor_active;
    if (nk_console_is_active_widget(console)) {
        if (active) {
            console->context->style.progress.cursor_normal = cursor_active;
        }
        else {
            console->context->style.progress.cursor_normal = cursor_active;
        }
    }

    // Display the widget
    if (nk_progress(console->context, data->value_size, data->max_size, nk_true)) {
        if (console->onchange != NULL) {
            console->onchange(console);
        }
    }

    // Restore the styles
    console->context->style.progress.cursor_normal = cursor_normal;
    console->context->style.progress.cursor_hover = cursor_hover;
    console->context->style.progress.cursor_active = cursor_active;

    if (console->disabled) {
        nk_widget_disable_end(console->context);
    }

    // Allow switching up/down in widgets
    if (nk_console_is_active_widget(console)) {
        nk_console_check_up_down(console, widget_bounds);
        nk_console_check_tooltip(console);
    }

    return widget_bounds;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_PROGRESS_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
