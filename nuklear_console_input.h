#ifndef NK_CONSOLE_INPUT_H__
#define NK_CONSOLE_INPUT_H__

/**
 * A keyboard key captured by an input widget, stored in an nk_rune as an ASCII-based code. Both a typed character and a Nuklear special key (enum nk_keys) resolve onto a single number line:
 *   - NK_CONSOLE_KEY_NONE
 *   - Control keys ASCII codes (8, 9, 13, 27, 127)
 *   - Printable characters Unicode codepoint (32 .. 0x10FFFF. 'A' = 65)
 *   - Keys with no ASCII equivalent: NK_CONSOLE_KEY_SPECIAL + the enum nk_keys value, a reserved range past Unicode's max.
 *
 * @see nk_console_input_key_from_keys()
 * @see nk_console_input_key_to_keys()
 * @see nk_console_input_key()
 */

#define NK_CONSOLE_KEY_NONE        0u /* Nothing */
#define NK_CONSOLE_KEY_BACKSPACE   8u /* ASCII BS  */
#define NK_CONSOLE_KEY_TAB         9u /* ASCII HT  */
#define NK_CONSOLE_KEY_ENTER       13u /* ASCII CR  */
#define NK_CONSOLE_KEY_ESCAPE      27u /* ASCII ESC */
#define NK_CONSOLE_KEY_SPACE       32u /* ASCII SP  */
#define NK_CONSOLE_KEY_DELETE      127u /* ASCII DEL */

/* Printable characters occupy 32 .. 0x10FFFF (stored as their codepoint). */

/* Keys with no ASCII equivalent live one past Unicode's last codepoint. */
#define NK_CONSOLE_KEY_SPECIAL     0x110000u
#define NK_CONSOLE_KEY_UP          (NK_CONSOLE_KEY_SPECIAL + NK_KEY_UP)
#define NK_CONSOLE_KEY_DOWN        (NK_CONSOLE_KEY_SPECIAL + NK_KEY_DOWN)
#define NK_CONSOLE_KEY_LEFT        (NK_CONSOLE_KEY_SPECIAL + NK_KEY_LEFT)
#define NK_CONSOLE_KEY_RIGHT       (NK_CONSOLE_KEY_SPECIAL + NK_KEY_RIGHT)
#define NK_CONSOLE_KEY_SHIFT       (NK_CONSOLE_KEY_SPECIAL + NK_KEY_SHIFT)
#define NK_CONSOLE_KEY_CTRL        (NK_CONSOLE_KEY_SPECIAL + NK_KEY_CTRL)
#define NK_CONSOLE_KEY_ALT         (NK_CONSOLE_KEY_SPECIAL + NK_KEY_ALT)
#define NK_CONSOLE_KEY_COPY        (NK_CONSOLE_KEY_SPECIAL + NK_KEY_COPY)
#define NK_CONSOLE_KEY_CUT         (NK_CONSOLE_KEY_SPECIAL + NK_KEY_CUT)
#define NK_CONSOLE_KEY_PASTE       (NK_CONSOLE_KEY_SPECIAL + NK_KEY_PASTE)
#define NK_CONSOLE_KEY_INSERT      (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_INSERT_MODE)
#define NK_CONSOLE_KEY_REPLACE     (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_REPLACE_MODE)
#define NK_CONSOLE_KEY_HOME        (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_LINE_START)
#define NK_CONSOLE_KEY_END         (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_LINE_END)
#define NK_CONSOLE_KEY_TEXT_START  (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_START)
#define NK_CONSOLE_KEY_TEXT_END    (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_END)
#define NK_CONSOLE_KEY_UNDO        (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_UNDO)
#define NK_CONSOLE_KEY_REDO        (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_REDO)
#define NK_CONSOLE_KEY_SELECT_ALL  (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_SELECT_ALL)
#define NK_CONSOLE_KEY_WORD_LEFT   (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_WORD_LEFT)
#define NK_CONSOLE_KEY_WORD_RIGHT  (NK_CONSOLE_KEY_SPECIAL + NK_KEY_TEXT_WORD_RIGHT)
#define NK_CONSOLE_KEY_SCROLL_START (NK_CONSOLE_KEY_SPECIAL + NK_KEY_SCROLL_START)
#define NK_CONSOLE_KEY_SCROLL_END  (NK_CONSOLE_KEY_SPECIAL + NK_KEY_SCROLL_END)
#define NK_CONSOLE_KEY_SCROLL_DOWN (NK_CONSOLE_KEY_SPECIAL + NK_KEY_SCROLL_DOWN)
#define NK_CONSOLE_KEY_SCROLL_UP   (NK_CONSOLE_KEY_SPECIAL + NK_KEY_SCROLL_UP)
#define NK_CONSOLE_KEY_F1          (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F1)
#define NK_CONSOLE_KEY_F2          (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F2)
#define NK_CONSOLE_KEY_F3          (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F3)
#define NK_CONSOLE_KEY_F4          (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F4)
#define NK_CONSOLE_KEY_F5          (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F5)
#define NK_CONSOLE_KEY_F6          (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F6)
#define NK_CONSOLE_KEY_F7          (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F7)
#define NK_CONSOLE_KEY_F8          (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F8)
#define NK_CONSOLE_KEY_F9          (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F9)
#define NK_CONSOLE_KEY_F10         (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F10)
#define NK_CONSOLE_KEY_F11         (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F11)
#define NK_CONSOLE_KEY_F12         (NK_CONSOLE_KEY_SPECIAL + NK_KEY_F12)

/**
 * Flags that control which input sources are accepted by an input widget.
 *
 * @see nk_console_input_gamepad()
 */
typedef enum nk_console_input_flags {
    NK_CONSOLE_INPUT_FLAG_GAMEPAD = NK_FLAG(0), /**< Accept a gamepad button. @see out_gamepad_button */
    NK_CONSOLE_INPUT_FLAG_KEY     = NK_FLAG(1), /**< Accept a keyboard key (typed character or special key). @see out_key */
    NK_CONSOLE_INPUT_FLAG_MOUSE   = NK_FLAG(2), /**< Accept a mouse button. @see out_mouse_button */
} nk_console_input_flags;

/**
 * Data specifically used for the input widget.
 *
 * @see nk_console_input_gamepad()
 */
typedef struct nk_console_input_data {
    struct nk_console_button_data button_data; /** Inherited from button */
    int gamepad_number; /** The gamepad number of which to expect input from. Provide -1 for any gamepad. */
    int* out_gamepad_number; /** A pointer for where to store the gamepad number the button is associated with. */
    enum nk_gamepad_button* out_gamepad_button; /** A pointer to where to store the gamepad button. */
    float timer; /** A countdown timer to prompt the user with. @see NK_CONSOLE_INPUT_TIMER */
    enum nk_gamepad_button default_gamepad_button; /** Value assigned to out_gamepad_button on timeout. @see nk_console_input_set_gamepad_default */
    nk_rune default_key; /** Value assigned to out_key on timeout. @see nk_console_input_set_key_default */
    enum nk_buttons default_mouse_button; /** Value assigned to out_mouse_button on timeout. @see nk_console_input_set_mouse_default */
    nk_uint flags; /** Bitfield of nk_console_input_flags controlling accepted input sources. */
    nk_rune* out_key; /** Where to store a captured keyboard key (NK_CONSOLE_KEY_*). Only used when NK_CONSOLE_INPUT_FLAG_KEY is set. */
    enum nk_buttons* out_mouse_button; /** Where to store a captured mouse button. Only used when NK_CONSOLE_INPUT_FLAG_MOUSE is set. */
    nk_uint active; /** The nk_console_input_flags bit of the most recently captured source. 0 until a capture occurs. */
} nk_console_input_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Create a new input widget accepting any combination of gamepad, keyboard, and mouse.
 * Pass NULL for output pointers you don't need; flags are derived automatically.
 *
 * @param parent The parent console.
 * @param label The label to display.
 * @param gamepad_number Gamepad to listen on (-1 for any). Ignored when out_gamepad_button is NULL.
 * @param out_gamepad_number Where to store which gamepad was used. May be NULL.
 * @param out_gamepad_button Where to store the captured gamepad button. NULL to disable gamepad input.
 * @param out_key Where to store the captured keyboard key (NK_CONSOLE_KEY_*). NULL to disable keyboard input.
 * @param out_mouse_button Where to store the captured mouse button. NULL to disable mouse input.
 *
 * @return The new input widget.
 */
NK_API nk_console* nk_console_input(nk_console* parent, const char* label, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button, nk_rune* out_key, enum nk_buttons* out_mouse_button);

/** Create a gamepad-only input widget. Shorthand for nk_console_input() with key/mouse set to NULL. */
NK_API nk_console* nk_console_input_gamepad(nk_console* parent, const char* label, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button);

/** Create a keyboard-only input widget. Shorthand for nk_console_input() with gamepad/mouse set to NULL. */
NK_API nk_console* nk_console_input_key(nk_console* parent, const char* label, nk_rune* out_key);
#define nk_console_key(parent, label, out_key) nk_console_input_key(parent, label, out_key)

/** Create a mouse-only input widget. Shorthand for nk_console_input() with gamepad/key set to NULL. */
NK_API nk_console* nk_console_input_mouse(nk_console* parent, const char* label, enum nk_buttons* out_mouse_button);
/** Render the input-capture widget. @return The bounding rect. */
NK_API struct nk_rect nk_console_input_render(nk_console* widget);

/** Resolve a Nuklear special key (enum nk_keys) to its NK_CONSOLE_KEY_* value. */
NK_API nk_rune nk_console_input_key_from_keys(enum nk_keys key);

/** Port an NK_CONSOLE_KEY_* value back to an enum nk_keys; NK_KEY_NONE if it has no equivalent (e.g. a printable character). */
NK_API enum nk_keys nk_console_input_key_to_keys(nk_rune key);

/** Get the display name for a captured keyboard key (NK_CONSOLE_KEY_*). */
NK_API const char* nk_console_input_key_name(nk_rune key);
#define nk_console_key_name(key) nk_console_input_key_name(key)

/**
 * Set the default gamepad button assigned to out_gamepad_button when the capture prompt times out.
 * Defaults to NK_GAMEPAD_BUTTON_INVALID.
 */
NK_API void nk_console_input_set_gamepad_default(nk_console* widget, enum nk_gamepad_button gamepad_button);

/**
 * Set the default keyboard key assigned to out_key when the capture prompt times out.
 * Defaults to NK_CONSOLE_KEY_NONE.
 */
NK_API void nk_console_input_set_key_default(nk_console* widget, nk_rune key);

/**
 * Set the default mouse button assigned to out_mouse_button when the capture prompt times out.
 * Defaults to NK_BUTTON_LEFT.
 */
NK_API void nk_console_input_set_mouse_default(nk_console* widget, enum nk_buttons mouse_button);

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
 * Set the keyboard-key output pointer. Written when a key is captured with NK_CONSOLE_INPUT_FLAG_KEY.
 */
NK_API void nk_console_input_set_key_out(nk_console* widget, nk_rune* out_key);

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

/**
 * Determine which input source the widget is currently bound to.
 *
 * Prefers the most recently captured source (data->active); before any capture
 * it falls back to the highest-priority configured output pointer, in the order
 * key, mouse, gamepad.
 *
 * @return An nk_console_input_flags bit, or 0 when no output pointer is set.
 */
static nk_uint nk_console_input_active_source(const nk_console_input_data* data) {
    nk_uint flags;
    if (data == NULL) {
        return 0;
    }
    flags = data->flags != 0 ? data->flags : NK_CONSOLE_INPUT_FLAG_GAMEPAD;

    // Prefer the most recently captured source, when still accepted and stored.
    if ((flags & data->active) != 0) {
        switch (data->active) {
            case NK_CONSOLE_INPUT_FLAG_KEY:     if (data->out_key != NULL) return NK_CONSOLE_INPUT_FLAG_KEY; break;
            case NK_CONSOLE_INPUT_FLAG_MOUSE:   if (data->out_mouse_button != NULL) return NK_CONSOLE_INPUT_FLAG_MOUSE; break;
            case NK_CONSOLE_INPUT_FLAG_GAMEPAD: if (data->out_gamepad_button != NULL) return NK_CONSOLE_INPUT_FLAG_GAMEPAD; break;
            default: break;
        }
    }

    // Otherwise fall back to the highest-priority accepted, configured output.
    if ((flags & NK_CONSOLE_INPUT_FLAG_KEY) && data->out_key != NULL) return NK_CONSOLE_INPUT_FLAG_KEY;
    if ((flags & NK_CONSOLE_INPUT_FLAG_MOUSE) && data->out_mouse_button != NULL) return NK_CONSOLE_INPUT_FLAG_MOUSE;
    if ((flags & NK_CONSOLE_INPUT_FLAG_GAMEPAD) && data->out_gamepad_button != NULL) return NK_CONSOLE_INPUT_FLAG_GAMEPAD;
    return 0;
}

NK_API struct nk_rect nk_console_input_render(nk_console* console) {
    if (console == NULL || console->data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }
    nk_console_input_data* data = (nk_console_input_data*)console->data;

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

    // Determine the display label based on the active output mode. Reset the
    // symbol each frame so a leftover gamepad glyph clears when rebound.
    const char* display_label = "<None>";
    nk_uint active = nk_console_input_active_source(data);
    nk_console_button_set_symbol(console, NK_SYMBOL_NONE);
    if (active == NK_CONSOLE_INPUT_FLAG_KEY) {
        display_label = nk_console_input_key_name(*data->out_key);
    } else if (active == NK_CONSOLE_INPUT_FLAG_MOUSE) {
        switch (*data->out_mouse_button) {
            case NK_BUTTON_LEFT: display_label = "Left Mouse";   break;
            case NK_BUTTON_MIDDLE: display_label = "Middle Mouse"; break;
            case NK_BUTTON_RIGHT: display_label = "Right Mouse";  break;
            case NK_BUTTON_X1: display_label = "X1 Mouse"; break;
            case NK_BUTTON_X2: display_label = "Mouse X2"; break;
            default: display_label = "Mouse Button"; break;
        }
    } else if (active == NK_CONSOLE_INPUT_FLAG_GAMEPAD) {
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
                // Symbol already reset to NK_SYMBOL_NONE above.
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

    // Handle the timeout: apply the default for whichever source is active.
    if (data->timer >= NK_CONSOLE_INPUT_TIMER) {
        nk_uint source = nk_console_input_active_source(data);
        if (source == NK_CONSOLE_INPUT_FLAG_GAMEPAD && data->out_gamepad_button != NULL) {
            *data->out_gamepad_button = data->default_gamepad_button;
            data->active = NK_CONSOLE_INPUT_FLAG_GAMEPAD;
        } else if (source == NK_CONSOLE_INPUT_FLAG_KEY && data->out_key != NULL) {
            *data->out_key = data->default_key;
            data->active = NK_CONSOLE_INPUT_FLAG_KEY;
        } else if (source == NK_CONSOLE_INPUT_FLAG_MOUSE && data->out_mouse_button != NULL) {
            *data->out_mouse_button = data->default_mouse_button;
            data->active = NK_CONSOLE_INPUT_FLAG_MOUSE;
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

    // Blinking prompt label listing each accepted input source.
    if (((int)(data->timer * 2)) % 2 == 0) {
        // Longest prompt is "Press a Button, Key, or Mouse Button" (~37 chars);
        // 80 leaves headroom, and every copy below is bounded.
        char prompt[80];
        const char* sources[3];
        int source_count = 0;
        char* p = prompt;
        char* end = prompt + sizeof(prompt) - 1; // reserve room for the '\0'
        const char* word = "Press a ";
        int i;
        if (flags & NK_CONSOLE_INPUT_FLAG_GAMEPAD) sources[source_count++] = "Button";
        if (flags & NK_CONSOLE_INPUT_FLAG_KEY)     sources[source_count++] = "Key";
        if (flags & NK_CONSOLE_INPUT_FLAG_MOUSE)   sources[source_count++] = "Mouse Button";

        // Build "Press a A", "Press a A or B", or "Press a A, B, or C".
        while (*word != '\0' && p < end) *p++ = *word++;
        for (i = 0; i < source_count; i++) {
            if (i > 0) {
                const char* sep = (i == source_count - 1) ? (source_count > 2 ? ", or " : " or ") : ", ";
                while (*sep != '\0' && p < end) *p++ = *sep++;
            }
            word = sources[i];
            while (*word != '\0' && p < end) *p++ = *word++;
        }
        *p = '\0';
        nk_label(console->ctx, prompt, NK_TEXT_CENTERED);
    }
    else {
        nk_spacer(console->ctx);
    }

    // Check for input.
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    if (top_data->input_processed == nk_false) {
        // Keyboard and mouse are checked before the gamepad so that physical
        // keyboard/mouse input wins over a keyboard-backed virtual gamepad
        // (which maps characters such as 'a' or space onto gamepad buttons).
        // A capture records the active source rather than clearing the other
        // output pointers, so a multi-source widget stays re-bindable.

        // Keyboard key captured. A printable typed character (stored as its
        // codepoint) takes priority over special keys; control characters
        // (< 32) fall through to the special-key loop.
        if (!finished && (flags & NK_CONSOLE_INPUT_FLAG_KEY) && data->out_key != NULL) {
            if (console->ctx->input.keyboard.text_len > 0) {
                nk_rune ch = 0;
                nk_utf_decode(console->ctx->input.keyboard.text, &ch, console->ctx->input.keyboard.text_len);
                if (ch >= 32) {
                    *data->out_key = ch; // printable: store the codepoint directly
                    data->active = NK_CONSOLE_INPUT_FLAG_KEY;
                    nk_console_trigger_event(input, NK_CONSOLE_EVENT_CHANGED);
                    finished = nk_true;
                }
            }
            if (!finished) {
                int ki;
                for (ki = NK_KEY_NONE + 1; ki < NK_KEY_MAX; ki++) {
                    if (nk_input_is_key_released(&console->ctx->input, (enum nk_keys)ki)) {
                        *data->out_key = nk_console_input_key_from_keys((enum nk_keys)ki);
                        data->active = NK_CONSOLE_INPUT_FLAG_KEY;
                        nk_console_trigger_event(input, NK_CONSOLE_EVENT_CHANGED);
                        finished = nk_true;
                        break;
                    }
                }
            }
        }

        // Mouse button released.
        if (!finished && (flags & NK_CONSOLE_INPUT_FLAG_MOUSE) && data->out_mouse_button != NULL) {
            int mi;
            for (mi = NK_BUTTON_LEFT; mi < NK_BUTTON_MAX; mi++) {
                if (nk_input_is_mouse_released(&console->ctx->input, (enum nk_buttons)mi)) {
                    *data->out_mouse_button = (enum nk_buttons)mi;
                    data->active = NK_CONSOLE_INPUT_FLAG_MOUSE;
                    nk_console_trigger_event(input, NK_CONSOLE_EVENT_CHANGED);
                    finished = nk_true;
                    break;
                }
            }
        }

        // Gamepad button released.
        if (!finished && (flags & NK_CONSOLE_INPUT_FLAG_GAMEPAD) && data->out_gamepad_button != NULL) {
            if (nk_gamepad_any_button_released((struct nk_gamepads*)nk_console_get_gamepads(top), data->gamepad_number, data->out_gamepad_number, data->out_gamepad_button)) {
                data->active = NK_CONSOLE_INPUT_FLAG_GAMEPAD;
                nk_console_trigger_event(input, NK_CONSOLE_EVENT_CHANGED);
                finished = nk_true;
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

NK_API void nk_console_input_set_gamepad_default(nk_console* widget, enum nk_gamepad_button gamepad_button) {
    if (widget == NULL || widget->data == NULL) return;
    ((nk_console_input_data*)widget->data)->default_gamepad_button = gamepad_button;
}

NK_API void nk_console_input_set_key_default(nk_console* widget, nk_rune key) {
    if (widget == NULL || widget->data == NULL) return;
    ((nk_console_input_data*)widget->data)->default_key = key;
}

NK_API void nk_console_input_set_mouse_default(nk_console* widget, enum nk_buttons mouse_button) {
    if (widget == NULL || widget->data == NULL) return;
    ((nk_console_input_data*)widget->data)->default_mouse_button = mouse_button;
}

NK_API void nk_console_input_set_flags(nk_console* widget, nk_uint flags) {
    if (widget == NULL || widget->data == NULL) return;
    ((nk_console_input_data*)widget->data)->flags = flags;
}

NK_API nk_uint nk_console_input_get_flags(nk_console* widget) {
    nk_uint flags;
    if (widget == NULL || widget->data == NULL) return NK_CONSOLE_INPUT_FLAG_GAMEPAD;
    flags = ((nk_console_input_data*)widget->data)->flags;
    return flags != 0 ? flags : NK_CONSOLE_INPUT_FLAG_GAMEPAD;
}

NK_API void nk_console_input_set_key_out(nk_console* widget, nk_rune* out_key) {
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
    nk_console_input_data* data;
    if (widget == NULL || widget->data == NULL) return nk_false;
    data = (nk_console_input_data*)widget->data;
    if (data->active != 0) return data->active == NK_CONSOLE_INPUT_FLAG_GAMEPAD ? nk_true : nk_false;
    return nk_console_input_active_source(data) == NK_CONSOLE_INPUT_FLAG_GAMEPAD ? nk_true : nk_false;
}

NK_API nk_bool nk_console_input_is_key(nk_console* widget) {
    nk_console_input_data* data;
    if (widget == NULL || widget->data == NULL) return nk_false;
    data = (nk_console_input_data*)widget->data;
    if (data->active != 0) return data->active == NK_CONSOLE_INPUT_FLAG_KEY ? nk_true : nk_false;
    return nk_console_input_active_source(data) == NK_CONSOLE_INPUT_FLAG_KEY ? nk_true : nk_false;
}

NK_API nk_bool nk_console_input_is_mouse(nk_console* widget) {
    nk_console_input_data* data;
    if (widget == NULL || widget->data == NULL) return nk_false;
    data = (nk_console_input_data*)widget->data;
    if (data->active != 0) return data->active == NK_CONSOLE_INPUT_FLAG_MOUSE ? nk_true : nk_false;
    return nk_console_input_active_source(data) == NK_CONSOLE_INPUT_FLAG_MOUSE ? nk_true : nk_false;
}

NK_API nk_rune nk_console_input_key_from_keys(enum nk_keys key) {
    // The five keys with an ASCII code map to it; every other special key lands
    // in the reserved range as NK_CONSOLE_KEY_SPECIAL + its enum nk_keys value.
    switch (key) {
        case NK_KEY_BACKSPACE: return NK_CONSOLE_KEY_BACKSPACE;
        case NK_KEY_TAB: return NK_CONSOLE_KEY_TAB;
        case NK_KEY_ENTER: return NK_CONSOLE_KEY_ENTER;
        case NK_KEY_TEXT_RESET_MODE: return NK_CONSOLE_KEY_ESCAPE;
        case NK_KEY_DEL: return NK_CONSOLE_KEY_DELETE;
        default: break;
    }
    if (key > NK_KEY_NONE && key < NK_KEY_MAX) {
        return NK_CONSOLE_KEY_SPECIAL + (nk_rune)key;
    }
    return NK_CONSOLE_KEY_NONE;
}

NK_API enum nk_keys nk_console_input_key_to_keys(nk_rune key) {
    // Inverse of nk_console_input_key_from_keys(). Printable characters have no
    // enum nk_keys equivalent and resolve to NK_KEY_NONE.
    switch (key) {
        case NK_CONSOLE_KEY_BACKSPACE: return NK_KEY_BACKSPACE;
        case NK_CONSOLE_KEY_TAB: return NK_KEY_TAB;
        case NK_CONSOLE_KEY_ENTER: return NK_KEY_ENTER;
        case NK_CONSOLE_KEY_ESCAPE: return NK_KEY_TEXT_RESET_MODE;
        case NK_CONSOLE_KEY_DELETE: return NK_KEY_DEL;
        default: break;
    }
    if (key >= NK_CONSOLE_KEY_SPECIAL && (key - NK_CONSOLE_KEY_SPECIAL) < (nk_rune)NK_KEY_MAX) {
        return (enum nk_keys)(key - NK_CONSOLE_KEY_SPECIAL);
    }
    return NK_KEY_NONE;
}

NK_API const char* nk_console_input_key_name(nk_rune key) {
    static char nk_console_input_key_buf[NK_UTF_SIZE + 1];
    int len;
    switch (key) {
        case NK_CONSOLE_KEY_NONE: return "<None>";
        case NK_CONSOLE_KEY_BACKSPACE: return "Backspace";
        case NK_CONSOLE_KEY_TAB: return "Tab";
        case NK_CONSOLE_KEY_ENTER: return "Enter";
        case NK_CONSOLE_KEY_ESCAPE: return "Escape";
        case NK_CONSOLE_KEY_DELETE: return "Delete";
        case NK_CONSOLE_KEY_UP: return "Up";
        case NK_CONSOLE_KEY_DOWN: return "Down";
        case NK_CONSOLE_KEY_LEFT: return "Left";
        case NK_CONSOLE_KEY_RIGHT: return "Right";
        case NK_CONSOLE_KEY_SHIFT: return "Shift";
        case NK_CONSOLE_KEY_CTRL: return "Ctrl";
        case NK_CONSOLE_KEY_ALT: return "Alt";
        case NK_CONSOLE_KEY_COPY: return "Copy";
        case NK_CONSOLE_KEY_CUT: return "Cut";
        case NK_CONSOLE_KEY_PASTE: return "Paste";
        case NK_CONSOLE_KEY_INSERT: return "Insert";
        case NK_CONSOLE_KEY_REPLACE: return "Replace";
        case NK_CONSOLE_KEY_HOME: return "Home";
        case NK_CONSOLE_KEY_END: return "End";
        case NK_CONSOLE_KEY_TEXT_START: return "Ctrl+Home";
        case NK_CONSOLE_KEY_TEXT_END: return "Ctrl+End";
        case NK_CONSOLE_KEY_UNDO: return "Ctrl+Z";
        case NK_CONSOLE_KEY_REDO: return "Ctrl+Y";
        case NK_CONSOLE_KEY_SELECT_ALL: return "Ctrl+A";
        case NK_CONSOLE_KEY_WORD_LEFT: return "Ctrl+Left";
        case NK_CONSOLE_KEY_WORD_RIGHT: return "Ctrl+Right";
        case NK_CONSOLE_KEY_SCROLL_START: return "Scroll Start";
        case NK_CONSOLE_KEY_SCROLL_END: return "Scroll End";
        case NK_CONSOLE_KEY_SCROLL_DOWN: return "Scroll Down";
        case NK_CONSOLE_KEY_SCROLL_UP: return "Scroll Up";
        case NK_CONSOLE_KEY_F1: return "F1";
        case NK_CONSOLE_KEY_F2: return "F2";
        case NK_CONSOLE_KEY_F3: return "F3";
        case NK_CONSOLE_KEY_F4: return "F4";
        case NK_CONSOLE_KEY_F5: return "F5";
        case NK_CONSOLE_KEY_F6: return "F6";
        case NK_CONSOLE_KEY_F7: return "F7";
        case NK_CONSOLE_KEY_F8: return "F8";
        case NK_CONSOLE_KEY_F9: return "F9";
        case NK_CONSOLE_KEY_F10: return "F10";
        case NK_CONSOLE_KEY_F11: return "F11";
        case NK_CONSOLE_KEY_F12: return "F12";
        default: break; // fall through to printable-codepoint handling
    }
    if (key == 32) return "Space";
    if (key >= 32 && key <= 0x10FFFF) {
        len = nk_utf_encode((nk_rune)key, nk_console_input_key_buf, NK_UTF_SIZE);
        nk_console_input_key_buf[len] = '\0';
        return nk_console_input_key_buf;
    }
    return "Unknown";
}

NK_API nk_console* nk_console_input(nk_console* parent, const char* label, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button, nk_rune* out_key, enum nk_buttons* out_mouse_button) {
    nk_console_input_data* data;
    nk_console* widget;
    nk_console* active_state;
    if (parent == NULL) {
        return NULL;
    }

    data = (nk_console_input_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_input_data));
    nk_zero(data, sizeof(nk_console_input_data));
    data->gamepad_number = gamepad_number;
    data->out_gamepad_number = out_gamepad_number;
    data->out_gamepad_button = out_gamepad_button;
    data->default_gamepad_button = NK_GAMEPAD_BUTTON_INVALID;
    data->out_key = out_key;
    data->out_mouse_button = out_mouse_button;

    if (out_gamepad_button != NULL) data->flags |= NK_CONSOLE_INPUT_FLAG_GAMEPAD;
    if (out_key != NULL)            data->flags |= NK_CONSOLE_INPUT_FLAG_KEY;
    if (out_mouse_button != NULL)   data->flags |= NK_CONSOLE_INPUT_FLAG_MOUSE;

    widget = nk_console_label(parent, label);
    widget->type = NK_CONSOLE_INPUT;
    widget->columns = label == NULL ? 1 : 2;
    widget->render = nk_console_input_render;
    widget->selectable = nk_true;
    widget->data = data;

    active_state = nk_console_label(widget, NULL);
    active_state->type = NK_CONSOLE_INPUT_ACTIVE;
    active_state->render = nk_console_input_active_render;

    return widget;
}

NK_API nk_console* nk_console_input_gamepad(nk_console* parent, const char* label, int gamepad_number, int* out_gamepad_number, enum nk_gamepad_button* out_gamepad_button) {
    return nk_console_input(parent, label, gamepad_number, out_gamepad_number, out_gamepad_button, NULL, NULL);
}

NK_API nk_console* nk_console_input_key(nk_console* parent, const char* label, nk_rune* out_key) {
    return nk_console_input(parent, label, -1, NULL, NULL, out_key, NULL);
}

NK_API nk_console* nk_console_input_mouse(nk_console* parent, const char* label, enum nk_buttons* out_mouse_button) {
    return nk_console_input(parent, label, -1, NULL, NULL, NULL, out_mouse_button);
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_INPUT_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
