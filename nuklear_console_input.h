#ifndef NK_CONSOLE_INPUT_H__
#define NK_CONSOLE_INPUT_H__

/**
 * Flags that control which input sources are accepted by an input widget.
 *
 * @see nk_console_input()
 */
typedef enum nk_console_input_flags {
    NK_CONSOLE_INPUT_FLAG_GAMEPAD  = NK_FLAG(0),
    NK_CONSOLE_INPUT_FLAG_KEYBOARD = NK_FLAG(1),
    NK_CONSOLE_INPUT_FLAG_MOUSE    = NK_FLAG(2),
} nk_console_input_flags;

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
    enum nk_gamepad_button default_gamepad_button; /** Value assigned to out_gamepad_button on timeout. @see nk_console_input_set_default */
    nk_uint flags; /** Bitfield of nk_console_input_flags controlling accepted input sources. */
    nk_rune* out_key; /** Where to store a captured keyboard key. Only used when NK_CONSOLE_INPUT_FLAG_KEYBOARD is set. */
    enum nk_buttons* out_mouse_button; /** Where to store a captured mouse button. Only used when NK_CONSOLE_INPUT_FLAG_MOUSE is set. */
} nk_console_input_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Create a new input widget to get a gamepad button.
 *
 * @param parent The parent console of where to add the widget.
 * @param label The label to display.
 * @param gamepad_number The gamepad number to expect input from. Provide -1 for any gamepad.
 * @param out_gamepad_number When the user enters a button, this is where the gamepad number that was used to press the button.
 * @param out_gamepad_button When the user enters a button, this is where the gamepad button will be stored.
 *
 * @return The new input widget.
 */
NK_API nk_console* nk_console_input(nk_console* parent, const char* label, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button);
NK_API nk_console* nk_console_input_gamepad(nk_console* parent, const char* label, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button);
NK_API nk_console* nk_console_input_key(nk_console* parent, const char* label, nk_rune* out_key);
NK_API nk_console* nk_console_input_mouse(nk_console* parent, const char* label, enum nk_buttons* out_mouse_button);
NK_API struct nk_rect nk_console_input_render(nk_console* widget);
NK_API const char* nk_console_input_key_name(nk_rune key);

/**
 * Set the default gamepad button assigned to out_gamepad_button when the capture prompt times out.
 * Defaults to NK_GAMEPAD_BUTTON_INVALID.
 */
NK_API void nk_console_input_set_default(nk_console* widget, enum nk_gamepad_button gamepad_button);

/**
 * Get the default gamepad button assigned to out_gamepad_button when the capture prompt times out.
 */
NK_API enum nk_gamepad_button nk_console_input_get_default(nk_console* widget);

/**
 * Set the input source flags for the widget.
 * Use a bitwise OR of nk_console_input_flags values.
 * Defaults to NK_CONSOLE_INPUT_FLAG_GAMEPAD.
 */
NK_API void nk_console_input_set_flags(nk_console* widget, nk_uint flags);

/**
 * Get the current input source flags for the widget.
 */
NK_API nk_uint nk_console_input_get_flags(nk_console* widget);

/**
 * Set the keyboard output pointer. Called when a key is captured with NK_CONSOLE_INPUT_FLAG_KEYBOARD.
 */
NK_API void nk_console_input_set_keyboard_out(nk_console* widget, nk_rune* out_key);

/**
 * Set the mouse button output pointer. Called when a mouse button is captured with NK_CONSOLE_INPUT_FLAG_MOUSE.
 */
NK_API void nk_console_input_set_mouse_out(nk_console* widget, enum nk_buttons* out_mouse_button);

/**
 * Set the gamepad output pointers for the widget.
 */
NK_API void nk_console_input_set_gamepad_out(nk_console* widget, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button);

/**
 * Returns nk_true if the widget's active output is a gamepad button.
 */
NK_API nk_bool nk_console_input_is_gamepad(nk_console* widget);

/**
 * Returns nk_true if the widget's active output is a keyboard key.
 */
NK_API nk_bool nk_console_input_is_key(nk_console* widget);

/**
 * Returns nk_true if the widget's active output is a mouse button.
 */
NK_API nk_bool nk_console_input_is_mouse(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_INPUT_H__

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

#ifndef NK_CONSOLE_GAMEPAD
static const char* nk_gamepad_button_name(struct nk_gamepads* g, enum nk_gamepad_button b) {
    (void)g;
    static const char* names[] = {
        "Up","Down","Left","Right","A","B","X","Y","LB","RB","Back","Start","Guide"
    };
    if (b >= 0 && b < NK_GAMEPAD_BUTTON_LAST) return names[(int)b];
    return NULL;
}
static nk_bool nk_gamepad_any_button_released(struct nk_gamepads* g, int n, int* on, enum nk_gamepad_button* ob) {
    (void)g; (void)n; (void)on; (void)ob; return nk_false;
}
#endif /* !NK_CONSOLE_GAMEPAD */

NK_API struct nk_rect nk_console_input_render(nk_console* console) {
    if (console == NULL || console->data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }
    nk_console_input_data* data = (nk_console_input_data*)console->data;
    nk_uint flags = data->flags != 0 ? data->flags : NK_CONSOLE_INPUT_FLAG_GAMEPAD;

    // Require at least one valid output pointer.
    if (data->out_gamepad_button == NULL && data->out_key == NULL && data->out_mouse_button == NULL) {
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

    // Determine the display label based on the active output mode.
    const char* display_label = "<None>";
    if ((flags & NK_CONSOLE_INPUT_FLAG_KEYBOARD) && data->out_key != NULL) {
        display_label = nk_console_input_key_name(*data->out_key);
    } else if ((flags & NK_CONSOLE_INPUT_FLAG_MOUSE) && data->out_mouse_button != NULL) {
        switch (*data->out_mouse_button) {
            case NK_BUTTON_LEFT:   display_label = "Left Mouse";   break;
            case NK_BUTTON_MIDDLE: display_label = "Middle Mouse"; break;
            case NK_BUTTON_RIGHT:  display_label = "Right Mouse";  break;
            default:               display_label = "Mouse Button"; break;
        }
    } else if (data->out_gamepad_button != NULL) {
        // Display the mocked button symbol
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
        display_label = (*data->out_gamepad_button < 0) ? "<None>" : nk_gamepad_button_name(nk_console_get_gamepads(console), *data->out_gamepad_button);
        if (display_label == NULL) {
            display_label = "Unknown";
        }
    }

    // Switch the values to have the widget display as a button.
    int swap_columns = console->columns;
    const char* swap_label = console->label;
    int swap_label_length = console->label_length;
    console->columns = 0;
    console->label = display_label;
    console->label_length = 0;
    struct nk_rect widget_bounds = nk_console_button_render(console);
    console->columns = swap_columns;
    console->label = swap_label;
    console->label_length = swap_label_length;

    return widget_bounds;
}

/**
 * Go back as a post-render hook so that it handles the events safely afterwards.
 */
static void nk_console_input_back_post_render(nk_console* console, void* user_data) {
    NK_UNUSED(user_data);
    nk_console_button_back(console, NULL);
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
        if (data->out_gamepad_button != NULL) {
            *data->out_gamepad_button = data->default_gamepad_button;
        }
        finished = nk_true;
    }

    // Only display the timer if delta time is provided.
    if (data->timer > 0.0f) {
        // Display a progressbar, scaling the time to milliseconds.
        nk_prog(console->ctx, (size_t)(data->timer * 1000), (size_t)(NK_CONSOLE_INPUT_TIMER * 1000), nk_false);
    }

    // Determine the effective flags (default to gamepad for backward compat).
    nk_uint flags = data->flags != 0 ? data->flags : NK_CONSOLE_INPUT_FLAG_GAMEPAD;

    // Blinking prompt label based on active input sources.
    if (((int)(data->timer * 2)) % 2 == 0) {
        const char* prompt;
        if ((flags & (NK_CONSOLE_INPUT_FLAG_GAMEPAD | NK_CONSOLE_INPUT_FLAG_KEYBOARD | NK_CONSOLE_INPUT_FLAG_MOUSE)) == (NK_CONSOLE_INPUT_FLAG_GAMEPAD | NK_CONSOLE_INPUT_FLAG_KEYBOARD | NK_CONSOLE_INPUT_FLAG_MOUSE))
            prompt = "Press a Button, Key, or Mouse Button";
        else if ((flags & (NK_CONSOLE_INPUT_FLAG_KEYBOARD | NK_CONSOLE_INPUT_FLAG_MOUSE)) == (NK_CONSOLE_INPUT_FLAG_KEYBOARD | NK_CONSOLE_INPUT_FLAG_MOUSE))
            prompt = "Press a Key or Mouse Button";
        else if ((flags & (NK_CONSOLE_INPUT_FLAG_GAMEPAD | NK_CONSOLE_INPUT_FLAG_KEYBOARD)) == (NK_CONSOLE_INPUT_FLAG_GAMEPAD | NK_CONSOLE_INPUT_FLAG_KEYBOARD))
            prompt = "Press a Button or Key";
        else if ((flags & (NK_CONSOLE_INPUT_FLAG_GAMEPAD | NK_CONSOLE_INPUT_FLAG_MOUSE)) == (NK_CONSOLE_INPUT_FLAG_GAMEPAD | NK_CONSOLE_INPUT_FLAG_MOUSE))
            prompt = "Press a Button or Mouse Button";
        else if (flags & NK_CONSOLE_INPUT_FLAG_KEYBOARD)
            prompt = "Press a Key";
        else if (flags & NK_CONSOLE_INPUT_FLAG_MOUSE)
            prompt = "Press a Mouse Button";
        else
            prompt = "Press a Button";
        nk_label(console->ctx, prompt, NK_TEXT_CENTERED);
    }
    else {
        nk_spacer(console->ctx);
    }

    // Check for input.
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    if (top_data->input_processed == nk_false) {
        // Gamepad button released.
        if (!finished && (flags & NK_CONSOLE_INPUT_FLAG_GAMEPAD)) {
            if (nk_gamepad_any_button_released((struct nk_gamepads*)nk_console_get_gamepads(top), data->gamepad_number, data->out_gamepad_number, data->out_gamepad_button)) {
                data->out_key = NULL;
                data->out_mouse_button = NULL;
                nk_console_trigger_event(input, NK_CONSOLE_EVENT_CHANGED);
                finished = nk_true;
            }
        }

        // Keyboard key released.
        if (!finished && (flags & NK_CONSOLE_INPUT_FLAG_KEYBOARD)) {
            if (console->ctx->input.keyboard.text_len > 0) {
                nk_rune ch = 0;
                nk_utf_decode(console->ctx->input.keyboard.text, &ch, console->ctx->input.keyboard.text_len);
                if (ch >= 32 && data->out_key != NULL) {
                    *data->out_key = ch;
                    data->out_gamepad_button = NULL;
                    data->out_mouse_button = NULL;
                    nk_console_trigger_event(input, NK_CONSOLE_EVENT_CHANGED);
                    finished = nk_true;
                }
            }
            if (!finished) {
                int ki;
                for (ki = NK_KEY_NONE + 1; ki < NK_KEY_MAX; ki++) {
                    if (nk_input_is_key_released(&console->ctx->input, (enum nk_keys)ki)) {
                        if (data->out_key != NULL) *data->out_key = (nk_rune)ki;
                        data->out_gamepad_button = NULL;
                        data->out_mouse_button = NULL;
                        nk_console_trigger_event(input, NK_CONSOLE_EVENT_CHANGED);
                        finished = nk_true;
                        break;
                    }
                }
            }
        }

        // Mouse button released.
        if (!finished && (flags & NK_CONSOLE_INPUT_FLAG_MOUSE)) {
            int mi;
            for (mi = NK_BUTTON_LEFT; mi < NK_BUTTON_MAX; mi++) {
                if (nk_input_is_mouse_released(&console->ctx->input, (enum nk_buttons)mi)) {
                    if (data->out_mouse_button != NULL) *data->out_mouse_button = (enum nk_buttons)mi;
                    data->out_gamepad_button = NULL;
                    data->out_key = NULL;
                    nk_console_trigger_event(input, NK_CONSOLE_EVENT_CHANGED);
                    finished = nk_true;
                    break;
                }
            }
        }

        // Cancel on dismiss keys when only gamepad mode (backward compat).
        if (!finished && flags == NK_CONSOLE_INPUT_FLAG_GAMEPAD) {
            if (nk_input_is_key_released(&console->ctx->input, NK_KEY_BACKSPACE) || nk_input_is_key_released(&console->ctx->input, NK_KEY_ENTER) || nk_input_is_mouse_released(&console->ctx->input, NK_BUTTON_LEFT) || nk_input_is_mouse_released(&console->ctx->input, NK_BUTTON_RIGHT)) {
                finished = nk_true;
            }
        }
    }

    if (finished == nk_true) {
        top_data->input_processed = nk_true;
        data->timer = 0.0f;
        nk_console_add_event(console, NK_CONSOLE_EVENT_POST_RENDER_ONCE, &nk_console_input_back_post_render);
    }

    return nk_rect(0, 0, 0, 0);
}

NK_API void nk_console_input_set_default(nk_console* widget, enum nk_gamepad_button gamepad_button) {
    if (widget == NULL || widget->data == NULL) {
        return;
    }
    ((nk_console_input_data*)widget->data)->default_gamepad_button = gamepad_button;
}

NK_API enum nk_gamepad_button nk_console_input_get_default(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) {
        return NK_GAMEPAD_BUTTON_INVALID;
    }
    return ((nk_console_input_data*)widget->data)->default_gamepad_button;
}

NK_API void nk_console_input_set_flags(nk_console* widget, nk_uint flags) {
    if (widget == NULL || widget->data == NULL) return;
    ((nk_console_input_data*)widget->data)->flags = flags;
}

NK_API nk_uint nk_console_input_get_flags(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) return NK_CONSOLE_INPUT_FLAG_GAMEPAD;
    return ((nk_console_input_data*)widget->data)->flags;
}

NK_API void nk_console_input_set_keyboard_out(nk_console* widget, nk_rune* out_key) {
    if (widget == NULL || widget->data == NULL) return;
    ((nk_console_input_data*)widget->data)->out_key = out_key;
}

NK_API void nk_console_input_set_mouse_out(nk_console* widget, enum nk_buttons* out_mouse_button) {
    if (widget == NULL || widget->data == NULL) return;
    ((nk_console_input_data*)widget->data)->out_mouse_button = out_mouse_button;
}

NK_API void nk_console_input_set_gamepad_out(nk_console* widget, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button) {
    if (widget == NULL || widget->data == NULL) return;
    nk_console_input_data* data = (nk_console_input_data*)widget->data;
    data->gamepad_number = gamepad_number;
    data->out_gamepad_number = out_gamepad_number;
    data->out_gamepad_button = out_gamepad_button;
}

NK_API nk_bool nk_console_input_is_gamepad(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) return nk_false;
    return ((nk_console_input_data*)widget->data)->out_gamepad_button != NULL ? nk_true : nk_false;
}

NK_API nk_bool nk_console_input_is_key(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) return nk_false;
    return ((nk_console_input_data*)widget->data)->out_key != NULL ? nk_true : nk_false;
}

NK_API nk_bool nk_console_input_is_mouse(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) return nk_false;
    return ((nk_console_input_data*)widget->data)->out_mouse_button != NULL ? nk_true : nk_false;
}

NK_API const char* nk_console_input_key_name(nk_rune key) {
    if (key < (nk_rune)NK_KEY_MAX) {
        switch ((enum nk_keys)key) {
            case NK_KEY_NONE: return "<None>";
            case NK_KEY_SHIFT: return "Shift";
            case NK_KEY_CTRL: return "Ctrl";
            case NK_KEY_ALT: return "Alt";
            case NK_KEY_DEL: return "Delete";
            case NK_KEY_ENTER: return "Enter";
            case NK_KEY_TAB: return "Tab";
            case NK_KEY_BACKSPACE: return "Backspace";
            case NK_KEY_COPY: return "Copy";
            case NK_KEY_CUT: return "Cut";
            case NK_KEY_PASTE: return "Paste";
            case NK_KEY_UP: return "Up";
            case NK_KEY_DOWN: return "Down";
            case NK_KEY_LEFT: return "Left";
            case NK_KEY_RIGHT: return "Right";
            case NK_KEY_TEXT_INSERT_MODE: return "Insert";
            case NK_KEY_TEXT_REPLACE_MODE: return "Replace";
            case NK_KEY_TEXT_RESET_MODE: return "Escape";
            case NK_KEY_TEXT_LINE_START: return "Home";
            case NK_KEY_TEXT_LINE_END: return "End";
            case NK_KEY_TEXT_START: return "Ctrl+Home";
            case NK_KEY_TEXT_END: return "Ctrl+End";
            case NK_KEY_TEXT_UNDO: return "Ctrl+Z";
            case NK_KEY_TEXT_REDO: return "Ctrl+Y";
            case NK_KEY_TEXT_SELECT_ALL: return "Ctrl+A";
            case NK_KEY_TEXT_WORD_LEFT: return "Ctrl+Left";
            case NK_KEY_TEXT_WORD_RIGHT: return "Ctrl+Right";
            case NK_KEY_SCROLL_START: return "Scroll Start";
            case NK_KEY_SCROLL_END: return "Scroll End";
            case NK_KEY_SCROLL_DOWN: return "Scroll Down";
            case NK_KEY_SCROLL_UP: return "Scroll Up";
            case NK_KEY_F1: return "F1";
            case NK_KEY_F2: return "F2";
            case NK_KEY_F3: return "F3";
            case NK_KEY_F4: return "F4";
            case NK_KEY_F5: return "F5";
            case NK_KEY_F6: return "F6";
            case NK_KEY_F7: return "F7";
            case NK_KEY_F8: return "F8";
            case NK_KEY_F9: return "F9";
            case NK_KEY_F10: return "F10";
            case NK_KEY_F11: return "F11";
            case NK_KEY_F12: return "F12";
            default: return "Unknown";
        }
    }
    if (key == 32) return "Space";
    static char nk_console_input_key_char_buf[NK_UTF_SIZE + 1];
    int len = nk_utf_encode(key, nk_console_input_key_char_buf, NK_UTF_SIZE);
    nk_console_input_key_char_buf[len] = '\0';
    return nk_console_input_key_char_buf;
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
    data->default_gamepad_button = NK_GAMEPAD_BUTTON_INVALID;

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

NK_API nk_console* nk_console_input_gamepad(nk_console* parent, const char* label, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button) {
    return nk_console_input(parent, label, gamepad_number, out_gamepad_number, out_gamepad_button);
}

NK_API nk_console* nk_console_input_key(nk_console* parent, const char* label, nk_rune* out_key) {
    nk_console* widget = nk_console_input(parent, label, -1, NULL, NULL);
    if (widget == NULL) return NULL;
    nk_console_input_set_keyboard_out(widget, out_key);
    nk_console_input_set_flags(widget, NK_CONSOLE_INPUT_FLAG_KEYBOARD);
    return widget;
}

NK_API nk_console* nk_console_input_mouse(nk_console* parent, const char* label, enum nk_buttons* out_mouse_button) {
    nk_console* widget = nk_console_input(parent, label, -1, NULL, NULL);
    if (widget == NULL) return NULL;
    nk_console_input_set_mouse_out(widget, out_mouse_button);
    nk_console_input_set_flags(widget, NK_CONSOLE_INPUT_FLAG_MOUSE);
    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_INPUT_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
