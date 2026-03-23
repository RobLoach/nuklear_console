#ifndef NK_CONSOLE_TREE_H__
#define NK_CONSOLE_TREE_H__

typedef struct nk_console_tree_data {
    struct nk_console_button_data button; /** Inherited from button */
    struct nk_console** referenced_children; /** A list of the children this tree holds. */
} nk_console_tree_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Create a Tree widget.
 *
 * @param parent The parent widget.
 * @param label The label for the widget
 * @param expanded Whether or not the tree should begin as expanded.
 *
 * @return The created widget.
 */
NK_API nk_console* nk_console_tree(nk_console* parent, const char* label, nk_bool expanded);

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

static void nk_console_tree_event_clicked(nk_console* tree, void* user_data) {
    if (tree == NULL || tree->data == NULL) {
        return;
    }
    NK_UNUSED(user_data);
    nk_console_tree_data* data = (nk_console_tree_data*)tree->data;
    nk_console** referenced_children = data->referenced_children;

    nk_bool expanded;
    if (data->button.symbol == NK_SYMBOL_TRIANGLE_DOWN) {
        data->button.symbol = NK_SYMBOL_TRIANGLE_RIGHT;
        expanded = nk_false;
    }
    else {
        data->button.symbol = NK_SYMBOL_TRIANGLE_DOWN;
        expanded = nk_true;
    }

    for (size_t i = 0; i < cvector_size(referenced_children); i++) {
        if (referenced_children[i] != NULL) {
            referenced_children[i]->visible = expanded;
        }
    }

    // Update the scrollbar.
    nk_console* top = nk_console_get_top(tree);
    if (top == NULL || top->data == NULL) {
        return;
    }
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    top_data->scroll_requested = nk_true;

    struct nk_rect widget_bounds = nk_layout_widget_bounds(tree->ctx);
    nk_console_check_up_down(tree, widget_bounds);
}

static void nk_console_tree_event_destroyed(nk_console* tree, void* user_data) {
    if (tree == NULL || tree->data == NULL) {
        return;
    }
    NK_UNUSED(user_data);
    nk_console_tree_data* data = (nk_console_tree_data*)tree->data;
    if (data->referenced_children == NULL) {
        return;
    }

    cvector_free(data->referenced_children);
    data->referenced_children = NULL;
}

NK_API nk_console* nk_console_tree(nk_console* parent, const char* label, nk_bool expanded) {
    if (parent == NULL) {
        return NULL;
    }
    // Create the widget data.
    nk_console_tree_data* data = (nk_console_tree_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_tree_data));
    if (data == NULL) {
        return NULL;
    }
    nk_zero(data, sizeof(nk_console_tree_data));

    nk_console* widget = nk_console_label(parent, label);
    if (widget == NULL) {

        return NULL;
    }
    widget->type = NK_CONSOLE_TREE;
    widget->data = (void*)data;
    widget->selectable = nk_true;
    widget->columns = 1;
    widget->render = nk_console_button_render;
    data->button.symbol = expanded ? NK_SYMBOL_TRIANGLE_DOWN : NK_SYMBOL_TRIANGLE_RIGHT;
    nk_console_add_event(widget, NK_CONSOLE_EVENT_DESTROYED, &nk_console_tree_event_destroyed);
    nk_console_add_event(widget, NK_CONSOLE_EVENT_CLICKED, &nk_console_tree_event_clicked);

    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TREE_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
