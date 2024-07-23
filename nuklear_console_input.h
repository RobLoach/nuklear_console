#ifndef NK_CONSOLE_INPUT_H__
#define NK_CONSOLE_INPUT_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_input_data {
    struct nk_console_button_data button_data; /** Inherited from button */
    int* gamepad_number;
    enum nk_gamepad_button* gamepad_button;
    float timer;
    // TODO: Input: Add Keyboard/Mouse support?
} nk_console_input_data;

NK_API nk_console* nk_console_input(nk_console* parent, const char* label, int* gamepad_number, enum nk_gamepad_button* gamepad_button);
NK_API struct nk_rect nk_console_input_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_INPUT_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_INPUT_IMPLEMENTATION_ONCE
#define NK_CONSOLE_INPUT_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

static const char* nk_console_input_button_name(enum nk_gamepad_button button) {
    switch (button) {
        case NK_GAMEPAD_BUTTON_A: return "A";
        case NK_GAMEPAD_BUTTON_B: return "B";
        case NK_GAMEPAD_BUTTON_X: return "X";
        case NK_GAMEPAD_BUTTON_Y: return "Y";
        case NK_GAMEPAD_BUTTON_LB: return "Left Bumper";
        case NK_GAMEPAD_BUTTON_RB: return "Right Bumper";
        case NK_GAMEPAD_BUTTON_BACK: return "Back";
        case NK_GAMEPAD_BUTTON_START: return "Start";
        case NK_GAMEPAD_BUTTON_UP: return "Up";
        case NK_GAMEPAD_BUTTON_DOWN: return "Down";
        case NK_GAMEPAD_BUTTON_LEFT: return "Left";
        case NK_GAMEPAD_BUTTON_RIGHT: return "Right";
        default: return "Unknown";
    }
}

NK_API struct nk_rect nk_console_input_render(nk_console* console) {
    if (console == NULL || console->data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }
    nk_console_input_data* data = (nk_console_input_data*)console->data;
    if (data->gamepad_button == NULL) {
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

    // Display the mocked button
    switch (*data->gamepad_button) {
        case NK_GAMEPAD_BUTTON_UP:
            nk_console_button_set_symbol(console, NK_SYMBOL_TRIANGLE_UP);
            break;
        case NK_GAMEPAD_BUTTON_DOWN:
            nk_console_button_set_symbol(console, NK_SYMBOL_TRIANGLE_DOWN);
            break;
        case NK_GAMEPAD_BUTTON_LEFT:
            nk_console_button_set_symbol(console, NK_SYMBOL_TRIANGLE_LEFT);
            break;
        case NK_GAMEPAD_BUTTON_RIGHT:
            nk_console_button_set_symbol(console, NK_SYMBOL_TRIANGLE_RIGHT);
            break;
        case NK_GAMEPAD_BUTTON_START:
            nk_console_button_set_symbol(console, NK_SYMBOL_PLUS);
            break;
        case NK_GAMEPAD_BUTTON_BACK:
            nk_console_button_set_symbol(console, NK_SYMBOL_MINUS);
            break;
        default:
            nk_console_button_set_symbol(console, NK_SYMBOL_NONE);
            break;
    }

    int swap_columns = console->columns;
    const char* swap_label = console->label;
    int swap_label_length = console->label_length;
    console->columns = 0;
    console->label = nk_console_input_button_name(*data->gamepad_button);
    console->label_length = 0;
    struct nk_rect widget_bounds = nk_console_button_render(console);
    console->columns = swap_columns;
    console->label = swap_label;
    console->label_length = swap_label_length;

    return widget_bounds;
}

NK_API struct nk_rect nk_console_input_active_render(nk_console* console) {
    if (console == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    // Get the input widget.
    nk_console* input = console->parent;
    nk_console_input_data* data = (nk_console_input_data*)input->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(console);

    // Make sure we're at the top of the window.
    nk_window_set_scroll(console->ctx, 0, 0);

    // Display the label for the input
    nk_label(console->ctx, input->label, NK_TEXT_CENTERED);

    // Blinking Press a Button
    data->timer -= console->ctx->delta_time_seconds;
    if (data->timer >= 0.0f) {
        nk_label(console->ctx, "Press a Button", NK_TEXT_CENTERED);
    }
    else if (data->timer <= -0.5f) {
        data->timer = 1.0f;
    }

    // Await input.
    nk_console* top = nk_console_get_top(console);
    if (top->input_processed == nk_false) {
        if (nk_gamepad_any_button_pressed((struct nk_gamepads*)nk_console_get_gamepads(top), -1, data->gamepad_number, data->gamepad_button)) {
            top->input_processed = nk_true;
            nk_console_button_back(console);
        }
        else if (nk_input_is_key_pressed(&console->ctx->input, NK_KEY_BACKSPACE) || nk_input_is_key_pressed(&console->ctx->input, NK_KEY_ENTER)) {
            top->input_processed = nk_true;
            nk_console_button_back(console);
        }
        else if (nk_input_is_mouse_pressed(&console->ctx->input, NK_BUTTON_LEFT)) {
            top->input_processed = nk_true;
            nk_console_button_back(console);
        }
    }

    return nk_rect(0, 0, 0, 0);
}

NK_API nk_console* nk_console_input(nk_console* parent, const char* label, int* gamepad_number, enum nk_gamepad_button* gamepad_button) {
    if (parent == NULL) {
        return NULL;
    }

    // Create the widget data.
    nk_console_input_data* data = (nk_console_input_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_input_data));
    nk_zero(data, sizeof(nk_console_input_data));
    data->gamepad_number = gamepad_number;
    data->gamepad_button = gamepad_button;

    nk_console* widget = nk_console_label(parent, label);
    widget->type = NK_CONSOLE_INPUT;
    widget->columns = label == NULL ? 1 : 2;
    widget->render = nk_console_input_render;
    widget->selectable = nk_true;
    widget->data = data;

    // Set up the input state child.
    nk_console* active_state = nk_console_label(widget, NULL);
    active_state->type = NK_CONSOLE_INPUT_ACTIVE;
    active_state->render = nk_console_input_active_render;

    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_INPUT_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
