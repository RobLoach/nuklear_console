#ifndef NK_CONSOLE_KEY_H__
#define NK_CONSOLE_KEY_H__

/**
 * Data specifically used for the key widget.
 *
 * Stored value convention for out_key:
 *   0               = NK_KEY_NONE (nothing captured)
 *   1 .. NK_KEY_MAX = nk_keys special key (arrows, Enter, Backspace, etc.)
 *   > NK_KEY_MAX    = Unicode codepoint of a typed character
 *
 * @see nk_console_key()
 */
typedef struct nk_console_key_data {
    struct nk_console_button_data button_data; /** Inherited from button */
    nk_rune* out_key; /** A pointer to where to store the captured key/character. */
    float timer; /** A countdown timer to prompt the user with. @see NK_CONSOLE_KEY_TIMER */
} nk_console_key_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Create a new key widget to capture a keyboard key or typed character.
 *
 * When activated, the widget waits for the first keyboard input and stores it
 * in out_key using the following convention:
 *   0               = NK_KEY_NONE
 *   1 .. NK_KEY_MAX = nk_keys special key
 *   > NK_KEY_MAX    = Unicode codepoint of a typed character
 *
 * @param parent The parent console.
 * @param label The label to display.
 * @param out_key Where to store the captured key.
 *
 * @return The new key widget.
 */
NK_API nk_console* nk_console_key(nk_console* parent, const char* label, nk_rune* out_key);
NK_API struct nk_rect nk_console_key_render(nk_console* widget);
NK_API const char* nk_console_key_name(nk_rune key);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_KEY_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_KEY_IMPLEMENTATION_ONCE
#define NK_CONSOLE_KEY_IMPLEMENTATION_ONCE

#ifndef NK_CONSOLE_KEY_TIMER
/**
 * The amount of time to wait for a key before timing out, in seconds.
 */
#define NK_CONSOLE_KEY_TIMER 6.0f
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Get the display name for a key value produced by nk_console_key().
 *
 * Values 1..NK_KEY_MAX are nk_keys special keys. Values above NK_KEY_MAX are
 * Unicode codepoints; space is shown as "Space", others as their UTF-8 glyph.
 */
NK_API const char* nk_console_key_name(nk_rune key) {
    if (key < (nk_rune)NK_KEY_MAX) {
        switch ((enum nk_keys)key) {
            case NK_KEY_NONE: return "<None>";
            case NK_KEY_SHIFT: return "Shift";
            case NK_KEY_CTRL: return "Ctrl";
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
            case NK_KEY_TEXT_RESET_MODE: return "Reset Mode";
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
            // TODO: Add NK_KEY_F1 to NK_KEY_F12
            // TODO: Add ALT
            default: return "Unknown";
        }
    }

    /* Unicode character — encode into a static buffer and return it. */
    if (key == 32) return "Space";
    static char nk_console_key_char_buf[NK_UTF_SIZE + 1];
    int len = nk_utf_encode(key, nk_console_key_char_buf, NK_UTF_SIZE);
    nk_console_key_char_buf[len] = '\0';
    return nk_console_key_char_buf;
}

NK_API struct nk_rect nk_console_key_render(nk_console* console) {
    if (console == NULL || console->data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }
    nk_console_key_data* data = (nk_console_key_data*)console->data;
    if (data->out_key == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(console);

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

    /* Display the current key as a button. */
    int swap_columns = console->columns;
    const char* swap_label = console->label;
    int swap_label_length = console->label_length;
    console->columns = 0;
    console->label = nk_console_key_name(*data->out_key);
    console->label_length = 0;
    struct nk_rect widget_bounds = nk_console_button_render(console);
    console->columns = swap_columns;
    console->label = swap_label;
    console->label_length = swap_label_length;

    return widget_bounds;
}

/**
 * Render the "Press a Key" capture prompt.
 */
static struct nk_rect nk_console_key_active_render(nk_console* console) {
    if (console == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console* key_widget = console->parent;
    nk_console_key_data* data = (nk_console_key_data*)key_widget->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }
    nk_console* top = nk_console_get_top(console);

    nk_console_layout_widget(console);

    if (key_widget->label != NULL && key_widget->label[0] != '\0') {
        if (key_widget->label_length > 0) {
            nk_text(console->ctx, key_widget->label, key_widget->label_length, NK_TEXT_CENTERED);
        }
        else {
            nk_label(console->ctx, key_widget->label, NK_TEXT_CENTERED);
        }
    }

    if (data->out_key != NULL && *data->out_key != 0) {
        nk_label(console->ctx, nk_console_key_name(*data->out_key), NK_TEXT_CENTERED);
    }
    else {
        nk_spacer(console->ctx);
    }

    data->timer += console->ctx->delta_time_seconds;
    nk_bool finished = nk_false;

    if (data->timer >= NK_CONSOLE_KEY_TIMER) {
        finished = nk_true;
    }

    if (data->timer > 0.0f) {
        nk_prog(console->ctx, (size_t)(data->timer * 1000), (size_t)(NK_CONSOLE_KEY_TIMER * 1000), nk_false);
    }

    if (((int)(data->timer * 2)) % 2 == 0) {
        nk_label(console->ctx, "Press a Key", NK_TEXT_CENTERED);
    }
    else {
        nk_spacer(console->ctx);
    }

    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    if (top_data->input_processed == nk_false) {
        /* Typed character input takes priority over special keys. Control
         * characters (< 32) are excluded — they overlap with nk_keys values
         * and are handled by the special-key loop below. */
        if (console->ctx->input.keyboard.text_len > 0) {
            nk_rune ch = 0;
            nk_utf_decode(console->ctx->input.keyboard.text, &ch, console->ctx->input.keyboard.text_len);
            if (ch >= 32 && data->out_key != NULL) {
                *data->out_key = ch;
                nk_console_trigger_event(key_widget, NK_CONSOLE_EVENT_CHANGED);
                finished = nk_true;
            }
        }

        /* Special keys (arrows, Enter, Backspace, etc.).
         * Use released (not pressed) to match nk_console_button_pushed(A), so
         * that the same release that closes the prompt doesn't re-trigger the
         * button on the next frame after input_processed is cleared. */
        if (!finished) {
            int i;
            for (i = NK_KEY_NONE + 1; i < NK_KEY_MAX; i++) {
                if (nk_input_is_key_released(&console->ctx->input, (enum nk_keys)i)) {
                    if (data->out_key != NULL) {
                        *data->out_key = (nk_rune)i;
                    }
                    nk_console_trigger_event(key_widget, NK_CONSOLE_EVENT_CHANGED);
                    finished = nk_true;
                    break;
                }
            }
        }

        /* Cancel on mouse click without assigning. */
        if (!finished && (nk_input_is_mouse_pressed(&console->ctx->input, NK_BUTTON_LEFT) || nk_input_is_mouse_pressed(&console->ctx->input, NK_BUTTON_RIGHT))) {
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

NK_API nk_console* nk_console_key(nk_console* parent, const char* label, nk_rune* out_key) {
    if (parent == NULL) {
        return NULL;
    }

    nk_console_key_data* data = (nk_console_key_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_key_data));
    nk_zero(data, sizeof(nk_console_key_data));
    data->out_key = out_key;

    nk_console* widget = nk_console_label(parent, label);
    widget->type = NK_CONSOLE_KEY;
    widget->columns = label == NULL ? 1 : 2;
    widget->render = nk_console_key_render;
    widget->selectable = nk_true;
    widget->data = data;

    nk_console* active_state = nk_console_label(widget, NULL);
    active_state->type = NK_CONSOLE_KEY_ACTIVE;
    active_state->render = nk_console_key_active_render;

    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_KEY_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
