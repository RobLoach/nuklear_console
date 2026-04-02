#ifndef NK_CONSOLE_TEXTEDIT_TEXT_H__
#define NK_CONSOLE_TEXTEDIT_TEXT_H__

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * A TextEdit Text widget is a text field that is displayed as part of the on-screen keyboard.
 *
 * @see nk_console_textedit()
 * @internal
 */
NK_API nk_console* nk_console_textedit_text(nk_console* textedit);
NK_API struct nk_rect nk_console_textedit_text_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TEXTEDIT_TEXT_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_TEXTEDIT_TEXT_IMPLEMENTATION_ONCE
#define NK_CONSOLE_TEXTEDIT_TEXT_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_textedit_text_render(nk_console* widget) {
    if (widget == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    struct nk_rect widget_bounds = nk_layout_widget_bounds(widget->ctx);

    nk_console_layout_widget(widget);

    nk_console* textedit = widget->parent;
    nk_console_textedit_data* data = (nk_console_textedit_data*)textedit->data;
    nk_console_top_data* top_data = (nk_console_top_data*)nk_console_get_top(widget)->data;

    // Process checking the up/down switching in widgets before processing showing the widget itself
    if (nk_console_is_active_widget(widget) && top_data->input_processed == nk_false) {
        // Allow using ENTER to go back
        if (nk_console_button_pushed(widget, NK_GAMEPAD_BUTTON_A)) {
            top_data->input_processed = nk_true;
            nk_console_textedit_button_back_click(widget, NULL);
            return nk_rect(0, 0, 0, 0);
        }
        // Allow changing up/down only if they're not pressing backspace
        else if (!nk_input_is_key_pressed(&widget->ctx->input, NK_KEY_BACKSPACE)) {
            nk_console_check_up_down(widget);
        }

        // Display the tooltip for the textedit.
        nk_console_check_tooltip(textedit);
    }

    if (widget->disabled) {
        nk_widget_disable_begin(widget->ctx);
    }

    // Display the text edit
    if (nk_console_is_active_widget(widget)) {
        nk_edit_focus(widget->ctx, NK_EDIT_FIELD);
    }
    else {
        nk_edit_unfocus(widget->ctx);
    }

    // TODO: textedit_text: Add an option to change the filter.
    int buffer_strlen = data->buffer != NULL ? (int)nk_strlen(data->buffer) : 0;

    // Allow for masking the textedit.
    if (data->masked) {
        char mask[1024];
        int mask_max = data->buffer_size < (int)sizeof(mask) ? data->buffer_size : (int)sizeof(mask);
        for (int i = 0; i < buffer_strlen && i < mask_max - 1; i++) {
            mask[i] = '*';
        }
        mask[buffer_strlen < mask_max ? buffer_strlen : mask_max - 1] = '\0';
        nk_edit_string_zero_terminated(widget->ctx, NK_EDIT_FIELD, mask, mask_max, nk_filter_ascii);
        int mask_strlen = (int)nk_strlen(mask);
        if (mask_strlen < buffer_strlen) { // Characters deleted
            int real_len = (int)nk_strlen(data->buffer);
            int new_real_len = real_len - (buffer_strlen - mask_strlen);
            if (new_real_len < 0) {
                new_real_len = 0;
            }
            data->buffer[new_real_len] = '\0';
            nk_console_trigger_event(textedit, NK_CONSOLE_EVENT_CHANGED);
        } else if (mask_strlen > buffer_strlen) { // Characters added
            int real_len = (int)nk_strlen(data->buffer);
            for (int i = buffer_strlen; i < mask_strlen && real_len < data->buffer_size - 1; i++, real_len++) {
                data->buffer[real_len] = mask[i];
            }
            data->buffer[real_len] = '\0';
            nk_console_trigger_event(textedit, NK_CONSOLE_EVENT_CHANGED);
        }
    } else {
        nk_edit_string_zero_terminated(widget->ctx, NK_EDIT_FIELD, data->buffer, data->buffer_size, nk_filter_ascii);
        if (buffer_strlen != (int)nk_strlen(data->buffer)) {
            nk_console_trigger_event(textedit, NK_CONSOLE_EVENT_CHANGED);
        }
    }

    if (widget->disabled) {
        nk_widget_disable_end(widget->ctx);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_textedit_text(nk_console* parent) {
    nk_console* textedit_text = nk_console_label(parent, parent->label);
    textedit_text->type = NK_CONSOLE_TEXTEDIT_TEXT;
    textedit_text->parent = parent;
    textedit_text->alignment = NK_TEXT_LEFT;
    textedit_text->columns = 1;
    textedit_text->selectable = nk_true;
    textedit_text->render = nk_console_textedit_text_render;

    // Register the back event to unload the keyboard.
    nk_console_add_event(parent, NK_CONSOLE_EVENT_BACK, &nk_console_textedit_text_event_back);
    return textedit_text;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TEXTEDIT_TEXT_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
