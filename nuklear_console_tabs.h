#ifndef NK_CONSOLE_TABS_H__
#define NK_CONSOLE_TABS_H__

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Custom data for the tabs widget.
 */
typedef struct nk_console_tabs_data {
    int active_tab;
} nk_console_file_data;

NK_API nk_console* nk_console_tabs(nk_console* parent);
NK_API struct nk_rect nk_console_tabs_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TABS_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_TABS_IMPLEMENTATION_ONCE
#define NK_CONSOLE_TABS_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_tabs_render(nk_console* widget) {
    size_t count = cvector_size(widget->children);
    if (count == 0) {
        return nk_rect(0, 0, 0, 0);
    }

    if (nk_console)

    for (size_t i = 0; i < count; i++) {
        if (widget->children[i].type == NK_CONSOLE_TAB && widget->events[i].callback) {
            widget->children[i].callback(widget, widget->events[i].user_data);
            invoked = nk_true;
        }
    }


    // Since labels don't really have widget bounds, we get the bounds after the label is displayed as a work-around.
    struct nk_rect widget_bounds = nk_layout_widget_bounds(widget->ctx);

    return widget_bounds;
}

NK_API nk_console* nk_console_tabs(nk_console* parent) {
    if (parent == NULL) {
        return NULL;
    }

    // Create the widget data.
    nk_console_tabs_data* data = (nk_console_tabs_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_tabs_data));
    nk_zero(data, sizeof(nk_console_file_data));

    nk_handle handle;
    nk_console* tabs = (nk_console*)nk_console_malloc(handle, NULL, sizeof(nk_console));
    nk_zero(tabs, sizeof(nk_console));
    tabs->ctx = parent->ctx;
    tabs->type = NK_CONSOLE_TABS;
    tabs->parent = parent;
    tabs->columns = 0;
    tabs->render = nk_console_tabs_render;
    tabs->visible = nk_true;
    tabs->data = data;

    nk_console_add_child(parent, tabs);
    return tabs;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TABS_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
