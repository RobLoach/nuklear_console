#ifndef NK_CONSOLE_TEXTEDIT_TEXT_H__
#define NK_CONSOLE_TEXTEDIT_TEXT_H__

#if defined(__cplusplus)
extern "C" {
#endif

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

    struct nk_rect widget_bounds = nk_layout_widget_bounds(widget->context);

    if (widget->columns > 0) {
        nk_layout_row_dynamic(widget->context, 0, widget->columns);
    }

    nk_console* textedit = widget->parent;
    nk_console_textedit_data* data = (nk_console_textedit_data*)textedit->data;

    if (widget->disabled) {
        nk_widget_disable_begin(widget->context);
    }

    // Display the text edit
    if (nk_console_is_active_widget(widget)) {
        nk_edit_focus(widget->context, NK_EDIT_FIELD);
    }
    else {
        nk_edit_unfocus(widget->context);
    }

    nk_edit_string_zero_terminated(widget->context, NK_EDIT_FIELD, data->buffer, data->buffer_size, nk_filter_ascii);

    if (widget->disabled) {
        nk_widget_disable_end(widget->context);
    }

    // Allow switching up/down in widgets
    if (nk_console_is_active_widget(widget)) {

        // Allow using ENTER to go back
        if (nk_input_is_key_pressed(&widget->context->input, NK_KEY_ENTER)) {
            nk_console_get_top(widget)->input_processed = nk_true;
            nk_console_textedit_button_back_click(widget);
        }
        // Allow changing up/down only if it's not backspace
        else if (!nk_input_is_key_pressed(&widget->context->input, NK_KEY_BACKSPACE)) {
            nk_console_check_up_down(widget, widget_bounds);
            nk_console_check_tooltip(widget);
        }
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_textedit_text(nk_console* parent) {
    nk_console* textedit_text = nk_console_init(parent->context);
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
