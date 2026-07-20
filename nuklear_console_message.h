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

/**
 * Render a single message notification.
 */
NK_API void nk_console_message_render(nk_console* console, nk_console_message* message);

/**
 * Render all active messages for the console.
 */
NK_API void nk_console_render_message(nk_console* console);

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

    // Only add the message if it's not already in the queue.
    nk_console_message* end = (nk_console_message*)cvector_end(data->messages);
    for (nk_console_message* it = (nk_console_message*)cvector_begin(data->messages); it != end; it++) {
        if (nk_stricmpn(it->text, text, NK_CONSOLE_MESSAGE_MAX_LENGTH) == 0) {
            return;
        }
    }

    int len = nk_strlen(text);
    if (len > NK_CONSOLE_MESSAGE_MAX_LENGTH) {
        len = NK_CONSOLE_MESSAGE_MAX_LENGTH;
    }

    // Create the new message.
    nk_console_message message = {0};
    message.duration = NK_CONSOLE_MESSAGE_DURATION;
    NK_MEMCPY(message.text, text, (nk_size)len);
    message.text[len] = '\0';

    cvector_push_back(data->messages, message);
}

#ifndef NK_CONSOLE_MESSAGE_SCROLL_SPEED
#define NK_CONSOLE_MESSAGE_SCROLL_SPEED NK_CONSOLE_MARQUEE_SCROLL_SPEED
#endif
#ifndef NK_CONSOLE_MESSAGE_SCROLL_PAUSE
#define NK_CONSOLE_MESSAGE_SCROLL_PAUSE NK_CONSOLE_MARQUEE_SCROLL_PAUSE
#endif

/**
 * The eased slide fraction (0..1) of a message's in/out animation: 1 fully
 * off its edge, 0 resting on screen.
 *
 * The fraction is a pure function of the message's remaining duration, so
 * frames that report a zero delta time (e.g. SDL's millisecond tick
 * resolution at high frame rates) keep the same offset instead of snapping
 * the message to its resting position (#301). Backends without timing never
 * observe a delta, and their messages simply rest on screen.
 */
static float nk_console_message_slide_fraction(nk_console_top_data* data, nk_console_message* message) {
    if (data == NULL || message == NULL || data->message_time_observed == nk_false) {
        return 0.0f;
    }
    float t = 0.0f;
    if (message->duration <= 1.0f) {
        // Slide out: t goes 0->1 as duration drops from 1->0.
        t = 1.0f - message->duration;
    }
    else if (message->duration >= NK_CONSOLE_MESSAGE_DURATION - 1.0f) {
        // Slide in: t goes 1->0 as duration drops from DURATION->DURATION-1.
        t = message->duration - NK_CONSOLE_MESSAGE_DURATION + 1.0f;
    }
    if (t <= 0.0f) {
        return 0.0f;
    }
    // Smoothstep easing: t*t*(3-2t)
    return t * t * (3.0f - 2.0f * t);
}

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
    float t_s = nk_console_message_slide_fraction(data, message);
    if (t_s > 0.0f) {
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

    float tooltip_width = bounds.w - ctx->style.window.border;
    int text_len = nk_strlen(message->text);
    float full_text_width = ctx->style.font->width(ctx->style.font->userdata, ctx->style.font->height, message->text, text_len);
    nk_console_marquee_tooltip_render(ctx, message->text, text_len, full_text_width, tooltip_width, text_height, NK_CONSOLE_MESSAGE_SCROLL_SPEED, NK_CONSOLE_MESSAGE_SCROLL_PAUSE, &message->scroll_x);

    // Restore the mouse x/y positions.
    ctx->input.mouse.pos = mouse_pos;
}

NK_API void nk_console_render_message(nk_console* console) {
    nk_console_top_data* data = (nk_console_top_data*)console->data;

    // Remember that the backend provides timing, so message positioning stays
    // animated even on frames that report a zero delta.
    if (console->ctx->delta_time_seconds > 0) {
        data->message_time_observed = nk_true;
    }

    if (data->messages == NULL || cvector_empty(data->messages)) {
        return;
    }

    nk_console_message* it = &data->messages[0];

    // Advance duration or allow dismissal when delta time is unavailable.
    if (console->ctx->delta_time_seconds > 0) {
        it->duration -= console->ctx->delta_time_seconds;
    }
    else if (nk_console_button_pushed(console, NK_GAMEPAD_BUTTON_B)) {
        data->input_processed = nk_true;
        it->duration = 0.0f;
    }

    if (it->duration <= 0.0f) {
        cvector_erase(data->messages, 0);
        return;
    }

    nk_console_message_render(console, it);
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
