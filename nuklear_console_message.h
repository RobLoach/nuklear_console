#ifndef NK_CONSOLE_MESSAGE_H__
#define NK_CONSOLE_MESSAGE_H__

#if defined(__cplusplus)
extern "C" {
#endif

NK_API void nk_console_show_message(nk_console* console, enum nk_console_message_type type, const char* text);
NK_API void nk_console_render_messages(nk_console* console);

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

NK_API void nk_console_show_message(nk_console* console, enum nk_console_message_type type, const char* text) {
    if (console == NULL || text == NULL || text[0] == '\0') {
        return;
    }

    nk_console_top_data* data = (nk_console_top_data*)(nk_console_get_top(console)->data);
    if (data == NULL) {
        return;
    }

    // Create a new message
    nk_console_message message = {
        .duration = data->messages_default_duration,
        .type = type,
    };

    // Copy the string.
    for (int i = 0; i < 254; i++) {
        message.text[i] = text[i];
        if (text[i] == '\0') {
            break;
        }
    }
    message.text[255] = '\0'; // Make sure it's null-terminated

    // Make sure the starting duration is sane.
    if (data->messages_default_duration <= 0) {
        data->messages_default_duration = 3.0f;
    }

    cvector_push_back(data->messages, message);
}

NK_API void nk_console_render_message(nk_console* console, nk_console_message* message) {
    if (message == NULL) {
        return;
    }

    nk_console_top_data* data = (nk_console_top_data*)console->data;
    struct nk_context* ctx = console->ctx;
    if (data == NULL) {
        return;
    }

    const struct nk_style *style;
    struct nk_vec2 padding;

    style = &ctx->style;
    padding = style->window.padding;

    float text_height = (style->font->height + padding.y);
    int x = ctx->input.mouse.pos.x;
    int y = ctx->input.mouse.pos.y;

    // Display the tooltip at the bottom of the window, manipulating the mouse position
    struct nk_rect windowbounds = nk_window_get_bounds(ctx);
    ctx->input.mouse.pos.x = windowbounds.x;
    ctx->input.mouse.pos.y = windowbounds.y + windowbounds.h - text_height - padding.y * 2;

    if (nk_tooltip_begin(ctx, (float)windowbounds.w)) {
        nk_layout_row_dynamic(ctx, text_height, 1);
        
        nk_label(ctx, message->text, NK_TEXT_LEFT);
        nk_tooltip_end(ctx);
    }

    // Restore the mouse x/y positions.
    ctx->input.mouse.pos.x = x;
    ctx->input.mouse.pos.y = y;
}

NK_API void nk_console_render_messages(nk_console* console) {
    nk_console_top_data* data = (nk_console_top_data*)console->data;
    if (data->messages == NULL || cvector_size(data->messages) == 0 || console->ctx->delta_time_seconds <= 0.0f) {
        return;
    }

    nk_console_message* it;
    nk_bool clear_all = nk_true;

    nk_console_message* end = cvector_end(data->messages);
    for (it = cvector_begin(data->messages); it != end; ++it) {
        if (it->duration <= 0.0f) {
            continue;
        }

        it->duration -= console->ctx->delta_time_seconds;

        nk_console_render_message(console, it);
        clear_all = nk_false;
        break;
	}

    if (clear_all) {
        cvector_free(data->messages);
        data->messages = NULL;
    }
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_MESSAGE_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
