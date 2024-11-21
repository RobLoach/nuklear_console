#ifndef NK_CONSOLE_WINDOW_H__
#define NK_CONSOLE_WINDOW_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_window_data {
    struct nk_rect bounds;
    nk_flags flags;

    nk_console* active_parent; /** The parent that is currently being displayed. */

    /**
     * Message queue that is to be shown.
     */
    struct nk_console_message* messages;

    /**
     * When set, will determine where messages should appear in the window.
     */
    struct nk_rect message_bounds;

    /**
     * Custom user data. This is only applied to the window top console.
     *
     * @see nk_console_user_data()
     * @see nk_console_set_user_data()
     */
    void* user_data;

} nk_console_window_data;

NK_API nk_console* nk_console_window(nk_console* parent, const char* title, const struct nk_rect bounds, nk_flags flags);
NK_API struct nk_rect nk_console_window_render(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_WINDOW_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_WINDOW_IMPLEMENTATION_ONCE
#define NK_CONSOLE_WINDOW_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif


/**
 * Retrieve the first selectable widget from the given parent.
 *
 * @internal
 */
static nk_console* nk_console_find_first_selectable(nk_console* parent) {
    if (parent == NULL || parent->children == NULL) {
        return NULL;
    }

    // Iterate through the children to find the first selectable widget.
    for (size_t i = 0; i < cvector_size(parent->children); i++) {
        if (parent->children[i] != NULL) {
            if (nk_console_selectable(parent->children[i])) {
                return parent->children[i];
            }
        }
    }

    return NULL;
}

/**
 * Check that all children of the given console have the same type.
 *
 * All children are windows or all children are widgets, mixing is not allowed.
 */
static nk_bool nk_console_has_consistent_children(nk_console* top) {
    NK_ASSERT(top != NULL);
    NK_ASSERT(top->parent == NULL);

    if (cvector_size(top->children) == 0) {
        return nk_true; // No children, so it satisfies the condition
    }

    // Determine the type of the first child to establish the rule
    nk_console_widget_type first_type = top->children[0]->type;

    // Verify that all children have the same type
    for (size_t i = 1; i < cvector_size(top->children); i++) {
        if (top->children[i]->type != first_type) {
            return nk_false; // Mismatch found, condition not satisfied
        }
    }

    return nk_true; // All children are consistent
}

/**
 * Capture the window from the nk context.
 */
static struct nk_console* nk_console_capture_window(nk_console* console) {
    NK_ASSERT(console->ctx->current != NULL); /** No window is currently active. */

    // Get current window
    struct nk_window* nk_win = console->ctx->current;

    // Store ptr to widgets
    struct nk_console** children = console->children;

    // Clear top children
    console->children = NULL;

    // Add a single window to the top console
    nk_console* window = nk_console_window(console, nk_win->name_string, nk_win->bounds, nk_win->flags);

    // Add widgets to window
    window->children = children;

    // Set window as parent for all window children
    for (size_t i = 0; i < cvector_size(window->children); ++i) {
        window->children[i]->parent = window;
    }

    return window;
}


/**
 * Render the contents of a console window.
 */
NK_API struct nk_rect nk_console_window_render(nk_console* console) {
    nk_console_window_data* data = (nk_console_window_data*)console->data;

    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    // Render the active message.
    nk_console_render_message(console);

    // Render all of the parent's children.
    if (data->active_parent->children != NULL) {
        // Make sure there's an active widget selected.
        if (data->active_parent->activeWidget == NULL) {
            nk_console_set_active_widget(nk_console_find_first_selectable(data->active_parent));
        }
        else {
            // Make sure the widget actually exists and can be selected.
            nk_bool widgetFound = nk_false;
            for (size_t i = 0; i < cvector_size(data->active_parent->children); ++i) {
                if (data->active_parent->children[i] == data->active_parent->activeWidget) {
                    // Ensure the widget is still selectable.
                    if (nk_console_selectable(data->active_parent->activeWidget)) {
                        widgetFound = nk_true;
                    }
                    break;
                }
            }
            if (widgetFound == nk_false) {
                nk_console_set_active_widget(nk_console_find_first_selectable(data->active_parent));
            }
        }

        // Render all the children
        for (size_t i = 0; i < cvector_size(data->active_parent->children); ++i) {
            nk_console_render(data->active_parent->children[i]);
        }
    }

    // Invoke the post-render events.
    size_t count = cvector_size(console->events);
    if (count > 0) {
        nk_bool can_erase = nk_true;
        for (size_t i = 0; i < count; i++) {
            if (console->events[i].type == NK_CONSOLE_EVENT_POST_RENDER_ONCE) {
                // Call the callback if it's set.
                if (console->events[i].callback != NULL) {
                    console->events[i].callback((nk_console*)console->events[i].user_data, NULL);
                    console->events[i].callback = NULL;
                }
            }
            else {
                can_erase = nk_false;
            }
        }
        if (can_erase) {
            cvector_clear(console->events);
        }
    }

    nk_console* top = nk_console_get_top(console);
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;

    if (!console->disabled && !top_data->input_processed) {
        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RB)) {
            int index = (top_data->active_window_index + 1) % (int)cvector_size(top->children);
            nk_console_set_active_window(top->children[index]);
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LB)) {
            int index = (top_data->active_window_index - 1 + (int)cvector_size(top->children)) % (int)cvector_size(top->children);
            nk_console_set_active_window(top->children[index]);
            top_data->input_processed = nk_true;
        }
    }

    return data->bounds;
}


/**
 * Add a window to console
 */
NK_API nk_console* nk_console_window(nk_console* top, const char* title, const struct nk_rect bounds, nk_flags flags) {
    NK_ASSERT(nk_console_has_consistent_children(top));

    nk_handle handle;

    nk_console_window_data* data = (nk_console_window_data*)nk_console_malloc(handle, NULL, sizeof(nk_console_window_data));
    nk_zero(data, sizeof(nk_console_window_data));

    data->bounds = bounds;
    data->flags = flags;

    nk_console* window = nk_console_label(top, title);
    window->data = (void*)data;
    window->type = NK_CONSOLE_WINDOW;
    window->selectable = nk_false;
    window->columns = 1;
    window->render = nk_console_window_render;

    data->active_parent = window;

    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    top_data->window_count++;

    return window;
}


#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_WINDOW_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
