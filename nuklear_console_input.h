#ifndef NK_CONSOLE_INPUT_H__
#define NK_CONSOLE_INPUT_H__

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Data specifically used for the input widget.
 *
 * @see nk_console_input()
 */
typedef struct nk_console_input_data {
    struct nk_console_button_data button_data; /** Inherited from button */
    int gamepad_number; /** The gamepad number of which to expect input from. Provied -1 for any gamepad. */
    int* out_gamepad_number; /** A pointer for where to store the gamepad number the button is associated with. */
    enum nk_gamepad_button* out_gamepad_button; /** A pointer to where to store the gamepad button. */
    float timer; /** A countdown timer to prompt the user with. @see NK_CONSOLE_INPUT_TIMER */
} nk_console_input_data;

/**
 * Create a new input widget to get a gamepad button.
 *
 * @param parent The parent console of where to add the widget.
 * @param label The label to display.
 * @param gamepad_number The gamepad number to expect input from. Provide -1 for any gamepad.
 * @param out_gamepad_number When the user enters a button, this is where the gamepad number that was used to press the button.
 * @param out_gamepad_button When the user enters a button, this is where the gamepad button will be stored.
 *
 * @todo Input: Add Keyboard support?
 *
 * @return The new input widget.
 */
NK_API nk_console* nk_console_input(nk_console* parent, const char* label, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button);
NK_API struct nk_rect nk_console_input_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_INPUT_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_INPUT_IMPLEMENTATION_ONCE
#define NK_CONSOLE_INPUT_IMPLEMENTATION_ONCE

#ifndef NK_CONSOLE_INPUT_TIMER
/**
 * The amount of time to wait for input before timing out, in seconds.
 */
#define NK_CONSOLE_INPUT_TIMER 6.0f
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Get the name of a gamepad button.
 *
 * @param button The button to get the name of.
 *
 * @return The name of the button.
 */
static const char* nk_console_input_button_name(enum nk_gamepad_button button) {
    switch (button) {
        case NK_GAMEPAD_BUTTON_INVALID: return "<None>";
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
    if (data->out_gamepad_button == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    // Set up the layout
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
    switch (*data->out_gamepad_button) {
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

    // Switch the values to have the widget display as a button.
    int swap_columns = console->columns;
    const char* swap_label = console->label;
    int swap_label_length = console->label_length;
    console->columns = 0;
    console->label = nk_console_input_button_name(*data->out_gamepad_button);
    console->label_length = 0;
    struct nk_rect widget_bounds = nk_console_button_render(console);
    console->columns = swap_columns;
    console->label = swap_label;
    console->label_length = swap_label_length;

    return widget_bounds;
}

/**
 * Render the "Press a Button" prompt.
 *
 * @param console The console to render the prompt for.
 *
 * @return An empty rectangle, because there isn't a widget to interact with.
 */
static struct nk_rect nk_console_input_active_render(nk_console* console) {
    if (console == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    // Get the parent input widget.
    nk_console* input = console->parent;
    nk_console_input_data* data = (nk_console_input_data*)input->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }
    nk_console* top = nk_console_get_top(console);

    // Set up the layout
    nk_console_layout_widget(console);

    // Display the label for the input
    if (input->label != NULL && input->label[0] != '\0') {
        if (input->label_length > 0) {
            nk_text(console->ctx, input->label, input->label_length, NK_TEXT_CENTERED);
        }
        else {
            nk_label(console->ctx, input->label, NK_TEXT_CENTERED);
        }
    }

    // Give a timer to the user.
    data->timer += console->ctx->delta_time_seconds;
    nk_bool finished = nk_false;

    // Handle the timeout
    if (data->timer >= NK_CONSOLE_INPUT_TIMER) {
        finished = nk_true;
    }

    // Only display the timer if delta time is provided.
    if (data->timer > 0.0f) {
        // Display a progressbar, scaling the time to milliseconds.
        nk_prog(console->ctx, (size_t)(data->timer * 1000), (size_t)(NK_CONSOLE_INPUT_TIMER * 1000), nk_false);
    }

    // Blinking press a button instructions
    if (((int)(data->timer * 2)) % 2 == 0) {
        nk_label(console->ctx, "Press a Button", NK_TEXT_CENTERED);
    }

    // Check for input.
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    if (top_data->input_processed == nk_false) {
        // Gamepad button pressed.
        if (nk_gamepad_any_button_pressed((struct nk_gamepads*)nk_console_get_gamepads(top), data->gamepad_number, data->out_gamepad_number, data->out_gamepad_button)) {
            // Trigger the onchange event and exit.
            nk_console_onchange(input);
            finished = nk_true;
        }
        // Any other input.
        else if (nk_input_is_key_pressed(&console->ctx->input, NK_KEY_BACKSPACE) ||
                nk_input_is_key_pressed(&console->ctx->input, NK_KEY_ENTER) ||
                nk_input_is_mouse_pressed(&console->ctx->input, NK_BUTTON_LEFT) ||
                nk_input_is_mouse_pressed(&console->ctx->input, NK_BUTTON_RIGHT)) {
            finished = nk_true;
        }
    }

    if (finished == nk_true) {
        top_data->input_processed = nk_true;
        data->timer = 0.0f;
        nk_console_button_back(console, NULL);
    }

    return nk_rect(0, 0, 0, 0);
}

NK_API nk_console* nk_console_input(nk_console* parent, const char* label, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button) {
    if (parent == NULL) {
        return NULL;
    }

    // Create the widget data.
    nk_console_input_data* data = (nk_console_input_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_input_data));
    nk_zero(data, sizeof(nk_console_input_data));
    data->gamepad_number = gamepad_number;
    data->out_gamepad_number = out_gamepad_number;
    data->out_gamepad_button = out_gamepad_button;

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
