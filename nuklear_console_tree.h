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
 * @param expanded The starting state of whether or not the tree should be expanded.
 *
 * @return The created widget.
 */
NK_API nk_console* nk_console_tree(nk_console* parent, const char* label, nk_bool expanded);

/**
 * Retrieves whether or not the given tree is expanded.
 *
 * @param tree The tree widget to check against.
 * @return True if the tree is opened, false otherwise.
 */
NK_API nk_bool nk_console_tree_expanded(nk_console* tree);

/**
 * Expand or collapse the given tree.
 *
 * @param tree The tree widget to expand/collapse.
 * @param expanded True to expand the tree, false to collapse.
 * @return True if the tree is now opened, false otherwise.
 */
NK_API nk_bool nk_console_tree_set_expanded(nk_console* tree, nk_bool expanded);

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

NK_API nk_bool nk_console_tree_expanded(nk_console* tree) {
    if (tree == NULL || tree->data == NULL || tree->type != NK_CONSOLE_TREE) {
        return nk_false;
    }
    nk_console_tree_data* data = (nk_console_tree_data*)tree->data;
    return (data->button.symbol == NK_SYMBOL_TRIANGLE_DOWN) ? nk_true : nk_false;
}

/**
 * Applies the expansion or collapse of the tree.
 *
 * @param tree The tree to act on.
 * @param expanded True to expand the widget, false to collapse.
 */
static void nk_console_tree_apply_expanded(nk_console* tree, nk_bool expanded) {
    nk_console_tree_data* data = (nk_console_tree_data*)tree->data;
    data->button.symbol = expanded ? NK_SYMBOL_TRIANGLE_DOWN : NK_SYMBOL_TRIANGLE_RIGHT;

    // Toggle visibility of the children for the tree.
    nk_console** referenced_children = data->referenced_children;
    for (size_t i = 0; i < cvector_size(referenced_children); i++) {
        if (referenced_children[i] != NULL) {
            referenced_children[i]->visible = expanded;
        }
    }

    // Force the scrollbar to update if needed.
    nk_console* top = nk_console_get_top(tree);
    if (top != NULL && top->data != NULL) {
        nk_console_top_data* top_data = (nk_console_top_data*)top->data;
        top_data->scroll_to_widget = tree;
        top_data->scrollbar_required = nk_true;
    }
}

NK_API nk_bool nk_console_tree_set_expanded(nk_console* tree, nk_bool expanded) {
    if (tree == NULL || tree->data == NULL || tree->type != NK_CONSOLE_TREE) {
        return nk_false;
    }

    nk_bool current = nk_console_tree_expanded(tree);
    nk_console_tree_apply_expanded(tree, expanded);
    if (current != expanded) {
        nk_console_trigger_event(tree, NK_CONSOLE_EVENT_CHANGED);
    }
    return expanded;
}

static void nk_console_tree_event_clicked(nk_console* tree, void* user_data) {
    if (tree == NULL || tree->data == NULL) {
        return;
    }
    NK_UNUSED(user_data);
    nk_console_tree_apply_expanded(tree, !nk_console_tree_expanded(tree));
    nk_console_trigger_event(tree, NK_CONSOLE_EVENT_CHANGED);
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

    // Free up all the referenced children, removing them from the parent.
    for (size_t i = 0; i < cvector_size(data->referenced_children); i++) {
        nk_console* child = data->referenced_children[i];
        if (child == NULL) {
            continue;
        }

        // Find the child index in the parent.
        int index = nk_console_get_widget_index(child);
        if (index >= 0) {
            size_t size_index = (size_t)index;
            cvector_erase(child->parent->children, size_index);
        }

        // Free it up.
        nk_console_free(child);
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
        NK_CONSOLE_FREE(nk_handle_id(0), data);
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
