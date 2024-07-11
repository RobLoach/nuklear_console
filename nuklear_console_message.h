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

#endif  // NK_CONSOLE_MESSAGE_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_MESSAGE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_MESSAGE_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API void nk_console_show_message(nk_console* console, const char* text) {
    if (console == NULL || text == NULL || text[0] == '\0') {
        return;
    }

    nk_console_top_data* data = (nk_console_top_data*)(nk_console_get_top(console)->data);
    if (data == NULL) {
        return;
    }

    #ifndef NK_CONSOLE_MESSAGE_DEFAULT_DURATION
    #define NK_CONSOLE_MESSAGE_DEFAULT_DURATION 4.0f
    #endif  // NK_CONSOLE_MESSAGE_DEFAULT_DURATION

    // Make sure the starting duration is sane.
    if (data->messages_default_duration <= 0) {
        data->messages_default_duration = NK_CONSOLE_MESSAGE_DEFAULT_DURATION;
    }

    // Create a new message
    nk_console_message message = {
        .duration = data->messages_default_duration,
    };

    // Copy the string.
    for (int i = 0; i < 254; i++) {
        message.text[i] = text[i];
        if (text[i] == '\0') {
            break;
        }
    }
    message.text[255] = '\0'; // Make sure it's null-terminated

    cvector_push_back(data->messages, message);
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

    // Display the tooltip at the bottom of the window, by manipulating the mouse position
    struct nk_rect bounds = nk_window_get_bounds(ctx);
    bounds.w -= border;
    ctx->input.mouse.pos.x = bounds.x;
    ctx->input.mouse.pos.y = bounds.y + bounds.h - text_height - padding.y * 2 - border * 2.0f;

    // Animation, only if delta time is available.
    if (ctx->delta_time_seconds > 0) {
        if (message->duration <= 1.0f) {
            ctx->input.mouse.pos.y += (int)((1.0f - message->duration) * (text_height + padding.y * 2 + border * 2.0f));
        }
        else if (message->duration >= data->messages_default_duration - 1.0f) {
            ctx->input.mouse.pos.y += (int)((message->duration - data->messages_default_duration + 1.0f) * (text_height + padding.y * 2 + border * 2.0f));
        }
    }

    // Display the tooltip
    if (nk_tooltip_begin(ctx, (float)bounds.w)) {
        nk_layout_row_dynamic(ctx, text_height, 1);
        nk_label(ctx, message->text, NK_TEXT_LEFT);
        nk_tooltip_end(ctx);
    }

    // Restore the mouse x/y positions.
    ctx->input.mouse.pos = mouse_pos;
}

NK_API void nk_console_render_message(nk_console* console) {
    nk_console_top_data* data = (nk_console_top_data*)console->data;
    if (data->messages == NULL || cvector_size(data->messages) == 0) {
        return;
    }

    // Loop through all messages and display the first one.
    nk_bool clear_all = nk_true;
    nk_console_message* end = cvector_end(data->messages);
    for (nk_console_message* it = cvector_begin(data->messages); it != end; it++) {
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
            console->input_processed = nk_true;
            it->duration = 0.0f;
        }

        clear_all = nk_false;
        nk_console_message_render(console, it);
        break;
	}

    if (clear_all) {
        nk_console_free_messages(console);
    }
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_MESSAGE_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
