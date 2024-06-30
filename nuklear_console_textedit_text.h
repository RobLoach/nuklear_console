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

#endif  // NK_CONSOLE_TEXTEDIT_TEXT_H__

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

    // Process checking the up/down switching in widgets before processing showing the widget itself
    if (nk_console_is_active_widget(widget)) {
        // Allow using ENTER to go back
        if (nk_console_button_pushed(widget, NK_GAMEPAD_BUTTON_A)) {
            nk_console_get_top(widget)->input_processed = nk_true;
            nk_console_textedit_button_back_click(widget);
            return nk_rect(0, 0, 0, 0);
        }
        // Allow changing up/down only if they're not pressing backspace
        else if (!nk_input_is_key_pressed(&widget->ctx->input, NK_KEY_BACKSPACE)) {
            nk_console_check_up_down(widget, widget_bounds);
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
    // TODO: textedit_text: Trigger the onchange event when the text changes.
    nk_edit_string_zero_terminated(widget->ctx, NK_EDIT_FIELD, data->buffer, data->buffer_size, nk_filter_ascii);

    if (widget->disabled) {
        nk_widget_disable_end(widget->ctx);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_textedit_text(nk_console* parent) {
    nk_console* textedit_text = nk_console_init(parent->ctx);
    textedit_text->type = NK_CONSOLE_TEXTEDIT_TEXT;
    textedit_text->parent = parent;
    textedit_text->alignment = NK_TEXT_LEFT;
    textedit_text->columns = 1;
    textedit_text->selectable = nk_true;
    textedit_text->render = nk_console_textedit_text_render;
    cvector_push_back(parent->children, textedit_text);
    return textedit_text;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_TEXTEDIT_TEXT_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
