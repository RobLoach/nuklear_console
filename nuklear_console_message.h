#ifndef NK_CONSOLE_MESSAGE_H__
#define NK_CONSOLE_MESSAGE_H__

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Displays a notification message on the screen.
 *
 * @param console The active console system.
 * @param text The text to display.
 */
NK_API void nk_console_show_message(nk_console* console, const char* text);

/**
 * Set the bounding rectangle used to position messages.
 *
 * When the width is zero (the default), messages are placed relative to the current Nuklear window bounds. Set a non-zero rect to pin messages to a specific area of the screen instead.
 *
 * @param console Any widget within the console family.
 * @param bounds  The screen-space rect that messages should be anchored to.
 *
 * @see nk_console_get_message_bounds()
 */
NK_API void nk_console_set_message_bounds(nk_console* console, struct nk_rect bounds);

/**
 * Get the bounding rectangle currently used to position messages.
 *
 * @param console Any widget within the console family.
 * @return The rect set by nk_console_set_message_bounds(), or a zeroed rect when the default (window-relative) placement is active.
 *
 * @see nk_console_set_message_bounds()
 */
NK_API struct nk_rect nk_console_get_message_bounds(nk_console* console);

/**
 * Set the screen edge that messages slide in from.
 *
 * Defaults to NK_CONSOLE_MESSAGE_POSITION_BOTTOM.
 *
 * @param console  Any widget within the console family.
 * @param position The edge that messages should animate from.
 *
 * @see nk_console_get_message_position()
 */
NK_API void nk_console_set_message_position(nk_console* console, nk_console_message_position position);

/**
 * Get the screen edge that messages slide in from.
 *
 * @param console Any widget within the console family.
 * @return The position set by nk_console_set_message_position(), or
 *         NK_CONSOLE_MESSAGE_POSITION_BOTTOM by default.
 *
 * @see nk_console_set_message_position()
 */
NK_API nk_console_message_position nk_console_get_message_position(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_MESSAGE_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_MESSAGE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_MESSAGE_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef NK_CONSOLE_MESSAGE_DURATION
/**
 * A float determining how many seconds messages should be shown for.
 */
#define NK_CONSOLE_MESSAGE_DURATION 5.0f
#endif // NK_CONSOLE_MESSAGE_DURATION

NK_API void nk_console_show_message(nk_console* console, const char* text) {
    if (console == NULL || text == NULL || text[0] == '\0') {
        return;
    }

    nk_console_top_data* data = (nk_console_top_data*)(nk_console_get_top(console)->data);
    if (data == NULL) {
        return;
    }

    // Grab the length of the string, limit length to 255.
    int len = nk_strlen(text);
    if (len > 255) {
        len = 255;
    }

    // Only add the message if it's not already in the queue.
    nk_console_message* end = (nk_console_message*)cvector_end(data->messages);
    for (nk_console_message* it = (nk_console_message*)cvector_begin(data->messages); it != end; it++) {
        nk_bool same = nk_true;
        for (int i = 0; i <= len; i++) {
            if (it->text[i] != text[i]) {
                same = nk_false;
                break;
            }
        }
        if (same) {
            return;
        }
    }

    // Create the new message.
    nk_console_message message = {0};
    message.duration = NK_CONSOLE_MESSAGE_DURATION;
    NK_MEMCPY(message.text, text, (nk_size)len);
    message.text[len] = '\0'; // Make sure it's null-terminated

    // Add the new message to the message queue.
    cvector_push_back(data->messages, message);
}

#ifndef NK_CONSOLE_MESSAGE_SCROLL_SPEED
#define NK_CONSOLE_MESSAGE_SCROLL_SPEED NK_CONSOLE_MARQUEE_SCROLL_SPEED
#endif
#ifndef NK_CONSOLE_MESSAGE_SCROLL_PAUSE
#define NK_CONSOLE_MESSAGE_SCROLL_PAUSE NK_CONSOLE_MARQUEE_SCROLL_PAUSE
#endif

NK_API void nk_console_message_render(nk_console* console, nk_console_message* message) {
    if (message == NULL || console->data == NULL) {
        return;
    }

    // Retrieve style sizes.
    nk_console_top_data* data = (nk_console_top_data*)console->data;
    struct nk_context* ctx = console->ctx;
    struct nk_vec2 padding = ctx->style.window.padding;
    float border = ctx->style.window.border;
    float text_height = (ctx->style.font->height + padding.y);

    // Backup the mouse information as we'll be mocking it with a tooltip.
    struct nk_vec2 mouse_pos = ctx->input.mouse.pos;

    // Position the message against the configured screen edge by manipulating
    // the mouse position (the message is drawn as a tooltip).
    struct nk_rect bounds = data->message_bounds.w == 0 ? nk_window_get_bounds(ctx) : data->message_bounds;
    bounds.w -= border;
    nk_console_message_position position = data->message_position;
    float slide_h = text_height + padding.y * 2 + border * 2.0f;
    ctx->input.mouse.pos.x = bounds.x;
    ctx->input.mouse.pos.y = (position == NK_CONSOLE_MESSAGE_POSITION_BOTTOM) ? bounds.y + bounds.h - slide_h : bounds.y;

    // Slide the message off its anchored edge as it animates in and out.
    if (ctx->delta_time_seconds > 0) {
        float t = 0.0f;
        if (message->duration <= 1.0f) {
            // Slide out: t goes 0->1 as duration drops from 1->0.
            t = 1.0f - message->duration;
        }
        else if (message->duration >= NK_CONSOLE_MESSAGE_DURATION - 1.0f) {
            // Slide in: t goes 1->0 as duration drops from DURATION->DURATION-1.
            t = message->duration - NK_CONSOLE_MESSAGE_DURATION + 1.0f;
        }
        if (t > 0.0f) {
            // Smoothstep easing: t_s = t*t*(3-2t)
            float t_s = t * t * (3.0f - 2.0f * t);
            switch (position) {
                case NK_CONSOLE_MESSAGE_POSITION_TOP:
                    ctx->input.mouse.pos.y -= (int)(t_s * slide_h);
                    break;
                case NK_CONSOLE_MESSAGE_POSITION_LEFT:
                    ctx->input.mouse.pos.x -= (int)(t_s * bounds.w);
                    break;
                case NK_CONSOLE_MESSAGE_POSITION_RIGHT:
                    ctx->input.mouse.pos.x += (int)(t_s * bounds.w);
                    break;
                case NK_CONSOLE_MESSAGE_POSITION_BOTTOM:
                default:
                    ctx->input.mouse.pos.y += (int)(t_s * slide_h);
                    break;
            }
        }
    }

    float tooltip_width = bounds.w - ctx->style.window.border;
    int text_len = nk_strlen(message->text);
    float full_text_width = ctx->style.font->width(ctx->style.font->userdata, ctx->style.font->height, message->text, text_len);
    nk_console_marquee_tooltip_render(ctx, message->text, text_len, full_text_width,
        tooltip_width, text_height,
        NK_CONSOLE_MESSAGE_SCROLL_SPEED, NK_CONSOLE_MESSAGE_SCROLL_PAUSE,
        &message->scroll_x);

    // Restore the mouse x/y positions.
    ctx->input.mouse.pos = mouse_pos;
}

NK_API void nk_console_render_message(nk_console* console) {
    nk_console_top_data* data = (nk_console_top_data*)console->data;
    if (data->messages == NULL || cvector_empty(data->messages)) {
        return;
    }

    // Loop through all messages and display the first one.
    nk_bool clear_all = nk_true;
    nk_console_message* end = (nk_console_message*)cvector_end(data->messages);
    for (nk_console_message* it = (nk_console_message*)cvector_begin(data->messages); it != end; it++) {
        // Skip messages that have already been shown.
        if (it->duration <= 0.0f) {
            continue;
        }

        // Show only one message at a time.
        if (console->ctx->delta_time_seconds > 0) {
            it->duration -= console->ctx->delta_time_seconds;
        }
        // If animations arn't an option, allow dismissing the message.
        else if (nk_console_button_pushed(console, NK_GAMEPAD_BUTTON_B)) {
            data->state |= NK_CONSOLE_TOP_FLAG_INPUT_PROCESSED;
            it->duration = 0.0f;
        }

        clear_all = nk_false;
        nk_console_message_render(console, it);
        break;
    }

    if (clear_all) {
        cvector_clear(data->messages);
    }
}

NK_API void nk_console_set_message_bounds(nk_console* console, struct nk_rect bounds) {
    if (console == NULL) {
        return;
    }
    nk_console_top_data* data = (nk_console_top_data*)(nk_console_get_top(console)->data);
    if (data == NULL) {
        return;
    }
    data->message_bounds = bounds;
}

NK_API struct nk_rect nk_console_get_message_bounds(nk_console* console) {
    struct nk_rect zero = {0, 0, 0, 0};
    if (console == NULL) {
        return zero;
    }
    nk_console_top_data* data = (nk_console_top_data*)(nk_console_get_top(console)->data);
    if (data == NULL) {
        return zero;
    }
    return data->message_bounds;
}

NK_API void nk_console_set_message_position(nk_console* console, nk_console_message_position position) {
    if (console == NULL) {
        return;
    }
    nk_console_top_data* data = (nk_console_top_data*)(nk_console_get_top(console)->data);
    if (data == NULL) {
        return;
    }
    data->message_position = position;
}

NK_API nk_console_message_position nk_console_get_message_position(nk_console* console) {
    if (console == NULL) {
        return NK_CONSOLE_MESSAGE_POSITION_BOTTOM;
    }
    nk_console_top_data* data = (nk_console_top_data*)(nk_console_get_top(console)->data);
    if (data == NULL) {
        return NK_CONSOLE_MESSAGE_POSITION_BOTTOM;
    }
    return data->message_position;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_MESSAGE_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
