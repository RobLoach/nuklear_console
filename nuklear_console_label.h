#ifndef NK_CONSOLE_LABEL_H__
#define NK_CONSOLE_LABEL_H__

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_label(nk_console* parent, const char* text);
NK_API struct nk_rect nk_console_label_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_LABEL_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_LABEL_IMPLEMENTATION_ONCE
#define NK_CONSOLE_LABEL_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_label_render(nk_console* widget) {
    if (nk_strlen(widget->label) <= 0) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(widget);

    nk_bool selected = nk_false;
    if (!widget->disabled && widget->selectable) {
        selected = nk_console_is_active_widget(widget);
    }

    // Toggle it as disabled if needed.
    if (widget->disabled || (widget->selectable && !selected)) {
        nk_widget_disable_begin(widget->ctx);
    }

    // Display the label, considering the alignment.
    if (widget->alignment == NK_TEXT_LEFT) {
        nk_label_wrap(widget->ctx, widget->label);
    }
    else {
        nk_label(widget->ctx, widget->label, widget->alignment);
    }

    // Release the disabled state if needed.
    if (widget->disabled || (widget->selectable && !selected)) {
        nk_widget_disable_end(widget->ctx);
    }

    // Since labels don't really have widget bounds, we get the bounds after the label is displayed as a work-around.
    struct nk_rect widget_bounds = nk_layout_widget_bounds(widget->ctx);

    // Allow switching up/down in widgets
    if (selected) {
        if (nk_console_button_pushed(widget, NK_GAMEPAD_BUTTON_A) || nk_input_mouse_clicked(&widget->ctx->input, NK_BUTTON_LEFT, widget_bounds)) {
            nk_console_trigger_event(widget, NK_CONSOLE_EVENT_CLICKED);
        }
        else {
            nk_console_check_up_down(widget, widget_bounds);
        }
        nk_console_check_tooltip(widget);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_label(nk_console* parent, const char* text) {
    if (parent == NULL) {
        return NULL;
    }

    nk_handle handle;
    nk_console* label = (nk_console*)nk_console_malloc(handle, NULL, sizeof(nk_console));
    nk_zero(label, sizeof(nk_console));
    label->ctx = parent->ctx;
    label->alignment = NK_TEXT_ALIGN_CENTERED;
    label->type = NK_CONSOLE_LABEL;
    label->label = text;
    label->parent = parent;
    label->alignment = NK_TEXT_LEFT;
    label->columns = 1;
    label->render = nk_console_label_render;
    label->visible = nk_true;
    nk_console_add_child(parent, label);
    return label;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_LABEL_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
