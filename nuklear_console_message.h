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

#ifndef NK_CONSOLE_MARQUEE_SCROLL_SPEED
#define NK_CONSOLE_MARQUEE_SCROLL_SPEED 60.0f
#endif
#ifndef NK_CONSOLE_MARQUEE_SCROLL_PAUSE
#define NK_CONSOLE_MARQUEE_SCROLL_PAUSE 1.5f
#endif

/**
 * Advance a marquee scroll offset and return a pointer to the visible slice of text.
 * Returns `text` unchanged when the text fits or delta time is unavailable.
 * The caller owns `buf` (minimum `buf_size` bytes).
 */
static const char* nk_console_marquee_slice(
    struct nk_context* ctx,
    const char* text, int text_len,
    float full_text_width, float avail_width,
    float speed, float pause,
    float* scroll_x,
    char* buf, int buf_size)
{
    if (full_text_width <= avail_width || ctx->delta_time_seconds <= 0) {
        return text;
    }
    float pause_pixels = pause * speed;
    float total_cycle = full_text_width + pause_pixels;
    *scroll_x += ctx->delta_time_seconds * speed;
    if (*scroll_x > total_cycle) {
        *scroll_x -= total_cycle;
    }
    float offset = *scroll_x - pause_pixels;
    if (offset <= 0.0f) {
        return text;
    }
    int start = 0;
    for (int i = 1; i <= text_len; i++) {
        float w = ctx->style.font->width(ctx->style.font->userdata, ctx->style.font->height, text, i);
        if (w >= offset) { start = i - 1; break; }
        if (i == text_len) { start = text_len; }
    }
    int copy_len = text_len - start;
    if (copy_len >= buf_size) {
        copy_len = buf_size - 1;
    }
    NK_MEMCPY(buf, text + start, (nk_size)copy_len);
    buf[copy_len] = '\0';
    return buf;
}

/**
 * Render a single-line marquee tooltip of `tooltip_width` at the current mock mouse position.
 */
static void nk_console_tooltip_render_marquee(
    struct nk_context* ctx,
    const char* text, int text_len,
    float full_text_width,
    float tooltip_width, float text_height,
    float speed, float pause,
    float* scroll_x)
{
    float avail_width = tooltip_width - ctx->style.window.padding.x * 2.0f;
    char display_buf[256];
    const char* display_text = nk_console_marquee_slice(ctx, text, text_len,
        full_text_width, avail_width, speed, pause, scroll_x, display_buf, (int)sizeof(display_buf));
    struct nk_vec2 zero;
    nk_zero_struct(zero);
    if (nk_tooltip_begin_offset(ctx, tooltip_width, NK_TOP_LEFT, zero)) {
        nk_layout_row_dynamic(ctx, text_height, 1);
        nk_label(ctx, display_text, NK_TEXT_LEFT);
        nk_tooltip_end(ctx);
    }
}

#ifndef NK_CONSOLE_MESSAGE_DURATION
/**
 * A float determining how many seconds messages should be shown for.
 */
#define NK_CONSOLE_MESSAGE_DURATION 4.0f
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

    // Display the tooltip at the bottom of the window, by manipulating the mouse position
    struct nk_rect bounds = data->message_bounds.w == 0 ? nk_window_get_bounds(ctx) : data->message_bounds;
    bounds.w -= border;
    ctx->input.mouse.pos.x = bounds.x;
    ctx->input.mouse.pos.y = bounds.y + bounds.h - text_height - padding.y * 2 - border * 2.0f;

    // Animations can be applied if delta time is available.
    if (ctx->delta_time_seconds > 0) {
        // TODO(RobLoach): Apply easing to the message animation.
        if (message->duration <= 1.0f) {
            ctx->input.mouse.pos.y += (int)((1.0f - message->duration) * (text_height + padding.y * 2 + border * 2.0f));
        }
        else if (message->duration >= NK_CONSOLE_MESSAGE_DURATION - 1.0f) {
            ctx->input.mouse.pos.y += (int)((message->duration - NK_CONSOLE_MESSAGE_DURATION + 1.0f) * (text_height + padding.y * 2 + border * 2.0f));
        }
    }

    float tooltip_width = bounds.w - ctx->style.window.border;
    int text_len = nk_strlen(message->text);
    float full_text_width = ctx->style.font->width(ctx->style.font->userdata, ctx->style.font->height, message->text, text_len);
    nk_console_tooltip_render_marquee(ctx, message->text, text_len, full_text_width,
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
            data->input_processed = nk_true;
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

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_MESSAGE_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
