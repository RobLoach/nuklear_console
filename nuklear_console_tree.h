#ifndef NK_CONSOLE_TREE_H__
#define NK_CONSOLE_TREE_H__

typedef struct nk_console_tree_data {
    nk_bool* expanded;
    struct nk_console** owned_children; /** cvector; elements live in tree->parent->children */
} nk_console_tree_data;

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_tree(nk_console* parent, const char* text, nk_bool* expanded);
NK_API struct nk_rect nk_console_tree_render(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TREE_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_TREE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_TREE_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_tree_render(nk_console* console) {
    nk_console_tree_data* data = (nk_console_tree_data*)console->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(console);

    struct nk_rect widget_bounds = nk_layout_widget_bounds(console->ctx);
    nk_console* top = nk_console_get_top(console);
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;

    nk_bool active = nk_false;

    if (!console->disabled && nk_console_is_active_widget(console) && !top_data->input_processed) {
        nk_bool toggle   = nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_A);
        nk_bool expand   = nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT);
        nk_bool collapse = nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT);

        if (toggle || (expand && !*data->expanded) || (collapse && *data->expanded)) {
            if (toggle) {
                *data->expanded = !*data->expanded;
            }
            else {
                *data->expanded = expand ? nk_true : nk_false;
            }
            for (size_t i = 0; i < cvector_size(data->owned_children); i++) {
                data->owned_children[i]->visible = *data->expanded;
            }
            nk_console_trigger_event(console, NK_CONSOLE_EVENT_CHANGED);
            active = nk_true;
            top_data->input_processed = nk_true;
        }
    }

    // Style: match button hover/active styling
    struct nk_style_item buttonStyle = console->ctx->style.button.normal;
    struct nk_color textColor = console->ctx->style.button.text_normal;
    if (nk_console_is_active_widget(console)) {
        if (active) {
            console->ctx->style.button.normal = console->ctx->style.button.active;
            console->ctx->style.button.text_normal = console->ctx->style.button.text_active;
        }
        else {
            console->ctx->style.button.normal = console->ctx->style.button.hover;
            console->ctx->style.button.text_normal = console->ctx->style.button.text_hover;
        }
    }

    if (console->disabled) {
        nk_widget_disable_begin(console->ctx);
    }

    // Render: triangle symbol + label
    enum nk_symbol_type symbol = *data->expanded
        ? NK_SYMBOL_TRIANGLE_DOWN
        : NK_SYMBOL_TRIANGLE_RIGHT;
    nk_button_symbol_label(console->ctx, symbol, console->label, NK_TEXT_LEFT);

    // Restore styles
    console->ctx->style.button.normal = buttonStyle;
    console->ctx->style.button.text_normal = textColor;

    if (console->disabled) {
        nk_widget_disable_end(console->ctx);
    }

    if (nk_console_is_active_widget(console)) {
        nk_console_check_up_down(console, widget_bounds);
        nk_console_check_tooltip(console);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_tree(nk_console* parent, const char* text, nk_bool* expanded) {
    NK_ASSERT(expanded != NULL);

    nk_console_tree_data* data = (nk_console_tree_data*)NK_CONSOLE_MALLOC(
        nk_handle_id(0), NULL, sizeof(nk_console_tree_data));
    nk_zero(data, sizeof(nk_console_tree_data));

    nk_console* tree = nk_console_label(parent, text);
    tree->render = nk_console_tree_render;
    tree->type = NK_CONSOLE_TREE;
    tree->selectable = nk_true;
    tree->columns = 1;
    data->expanded = expanded;
    tree->data = (void*)data;
    return tree;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TREE_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
