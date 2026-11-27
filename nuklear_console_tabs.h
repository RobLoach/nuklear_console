#ifndef NK_CONSOLE_TABS_H__
#define NK_CONSOLE_TABS_H__

typedef struct nk_console_tabs_data {
    struct nk_console* selected; /** The selected tab, or NULL when there are no tabs. */
} nk_console_tabs_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Create a Tabs widget, which displays a row of tab headers above the widgets of the selected tab.
 *
 * The Tabs widget fills the rest of the screen, so it should be the last widget in its parent. While it's focused, switch between tabs
 * with LB/RB (Shift+Up/Down on keyboard), or by clicking a header. Each tab remembers its own focused widget.
 *
 * Tabs within a tab are not supported.
 *
 * @param parent The parent widget.
 *
 * @return The created widget.
 *
 * @see nk_console_tab()
 */
NK_API nk_console* nk_console_tabs(nk_console* parent);

/**
 * Add a tab to the given Tabs widget. Widgets added to the tab are only displayed while the tab is selected.
 *
 * @param tabs The Tabs widget.
 * @param label The label displayed in the tab header.
 *
 * @return The created tab, which widgets can be added to.
 */
NK_API nk_console* nk_console_tab(nk_console* tabs, const char* label);

/**
 * Get whether or not the given tab is the selected tab.
 *
 * @param tab The tab to check.
 *
 * @return True if the tab is selected, false otherwise.
 */
NK_API nk_bool nk_console_tabs_selected(nk_console* tab);

/**
 * Select the given tab.
 *
 * @param tab The tab to select.
 */
NK_API void nk_console_tabs_set_selected(nk_console* tab);

/**
 * Get the selected tab.
 *
 * @param tabs The Tabs widget.
 *
 * @return The selected tab, or NULL when there are no tabs.
 */
NK_API nk_console* nk_console_tabs_get_selected_tab(nk_console* tabs);

/**
 * Get the index of the selected tab.
 *
 * @param tabs The Tabs widget.
 *
 * @return The index of the selected tab, or -1 when there are no tabs.
 */
NK_API int nk_console_tabs_selected_index(nk_console* tabs);

/**
 * Select the tab at the given index.
 *
 * @param tabs The Tabs widget.
 * @param index The index of the tab to select.
 */
NK_API void nk_console_tabs_set_selected_index(nk_console* tabs, int index);

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

/**
 * Get the tab at the given index, or NULL if there isn't one.
 */
static nk_console* nk_console_tabs_get_tab(nk_console* tabs, int index) {
    if (index < 0 || index >= (int)cvector_size(tabs->children)) {
        return NULL;
    }
    nk_console* tab = tabs->children[index];
    return (tab != NULL && tab->type == NK_CONSOLE_TAB) ? tab : NULL;
}

/**
 * Make sure the tab has a selectable active widget, picking the first selectable one if needed.
 */
static void nk_console_tab_ensure_active_widget(nk_console* tab) {
    if (tab->activeWidget != NULL && nk_console_selectable(tab->activeWidget)) {
        return;
    }
    for (size_t i = 0; i < cvector_size(tab->children); i++) {
        if (tab->children[i] != NULL && nk_console_selectable(tab->children[i])) {
            nk_console_set_active_widget(tab->children[i]);
            return;
        }
    }
}

/**
 * Switch to the given tab, scrolling to its focused widget.
 */
static void nk_console_tabs_switch(nk_console* tabs, nk_console* tab) {
    nk_console_tabs_data* data = (nk_console_tabs_data*)tabs->data;
    if (tab == NULL || tab == data->selected) {
        return;
    }
    data->selected = tab;

    nk_console_tab_ensure_active_widget(tab);
    nk_console* top = nk_console_get_top(tabs);
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    top_data->scrollbar_required = nk_true;
    if (tab->activeWidget != NULL) {
        top_data->scroll_to_widget = tab->activeWidget;
    }
    else if (top->ctx != NULL && top->ctx->current != NULL) {
        nk_window_set_scroll(top->ctx, 0, 0);
    }

    nk_console_trigger_event(tabs, NK_CONSOLE_EVENT_CHANGED);
}

/**
 * Select the next or previous enabled tab, wrapping around the ends.
 */
static void nk_console_tabs_step(nk_console* tabs, int direction) {
    nk_console_tabs_data* data = (nk_console_tabs_data*)tabs->data;
    if (tabs->disabled) {
        return;
    }
    int count = (int)cvector_size(tabs->children);
    int index = data->selected != NULL ? nk_console_get_widget_index(data->selected) : (direction > 0 ? -1 : 0);
    for (int i = 0; i < count; i++) {
        index = (index + direction + count) % count;
        nk_console* tab = nk_console_tabs_get_tab(tabs, index);
        if (tab != NULL && tab->visible && !tab->disabled) {
            nk_console_tabs_switch(tabs, tab);
            return;
        }
    }
}

static void nk_console_tab_event_destroyed(nk_console* tab, void* user_data) {
    NK_UNUSED(user_data);

    // When the selected tab is freed, select the first remaining one. When the tabs widget itself is
    // being freed, its data is already gone. The tab is already detached from the tabs widget.
    nk_console* tabs = tab->parent;
    nk_console_tabs_data* data = tabs != NULL ? (nk_console_tabs_data*)tabs->data : NULL;
    if (data != NULL && data->selected == tab) {
        data->selected = NULL;
        for (int i = 0; i < (int)cvector_size(tabs->children) && data->selected == NULL; i++) {
            data->selected = nk_console_tabs_get_tab(tabs, i);
        }
    }
}

static struct nk_rect nk_console_tabs_render(nk_console* tabs) {
    NK_ASSERT(tabs != NULL);
    NK_ASSERT(tabs->data != NULL);
    nk_console_tabs_data* data = (nk_console_tabs_data*)tabs->data;
    struct nk_context* ctx = tabs->ctx;
    nk_bool focused = nk_console_is_active_widget(tabs);

    // LB/RB switch tabs, before the tab's widgets get a chance to page with them.
    nk_console* top = nk_console_get_top(tabs);
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    if (focused && !top_data->input_processed) {
        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LB)) {
            nk_console_tabs_step(tabs, -1);
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RB)) {
            nk_console_tabs_step(tabs, 1);
            top_data->input_processed = nk_true;
        }
    }

    // Tab headers.
    int visible_tabs = 0;
    for (int i = 0; i < (int)cvector_size(tabs->children); i++) {
        nk_console* tab = nk_console_tabs_get_tab(tabs, i);
        if (tab != NULL && tab->visible) {
            visible_tabs++;
        }
    }
    if (visible_tabs == 0) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_layout_row_dynamic(ctx, nk_console_layout_height(tabs), visible_tabs);
    struct nk_rect widget_bounds = nk_rect(0, 0, 0, 0);
    for (int i = 0; i < (int)cvector_size(tabs->children); i++) {
        nk_console* tab = nk_console_tabs_get_tab(tabs, i);
        if (tab == NULL || !tab->visible) {
            continue;
        }

        // Grow the bounds to cover all the headers.
        struct nk_rect bounds = nk_layout_widget_bounds(ctx);
        if (widget_bounds.w <= 0) {
            widget_bounds = bounds;
        }
        else {
            widget_bounds.w = bounds.x + bounds.w - widget_bounds.x;
        }

        nk_bool disabled = tabs->disabled || tab->disabled;
        if (disabled) {
            nk_widget_disable_begin(ctx);
        }
        nk_bool selected = (tab == data->selected) ? nk_true : nk_false;
        const char* label = tab->label != NULL ? tab->label : "";
        nk_bool clicked = tab->label_length > 0 ? nk_selectable_text(ctx, label, tab->label_length, NK_TEXT_CENTERED, &selected) : nk_selectable_label(ctx, label, NK_TEXT_CENTERED, &selected);
        if (disabled) {
            nk_widget_disable_end(ctx);
        }
        else if (clicked) {
            nk_console_tabs_switch(tabs, tab);
        }
    }

    // The selected tab's widgets. They're only focused while the tabs widget is.
    nk_console* tab = data->selected;
    if (tab != NULL) {
        nk_console* active = tab->activeWidget;
        if (focused) {
            nk_console_tab_ensure_active_widget(tab);
        }
        else {
            tab->activeWidget = NULL;
        }

        for (size_t i = 0; i < cvector_size(tab->children); i++) {
            nk_console_render(tab->children[i]);
        }

        // When the mouse moved onto one of the tab's widgets, focus the tabs widget too.
        if (!focused) {
            if (tab->activeWidget != NULL) {
                nk_console_set_active_widget(tabs);
            }
            else {
                tab->activeWidget = active;
            }
        }
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_tabs_get_selected_tab(nk_console* tabs) {
    NK_ASSERT(tabs != NULL);
    NK_ASSERT(tabs->data != NULL);
    NK_ASSERT(tabs->type == NK_CONSOLE_TABS);
    return ((nk_console_tabs_data*)tabs->data)->selected;
}

NK_API nk_bool nk_console_tabs_selected(nk_console* tab) {
    NK_ASSERT(tab != NULL);
    NK_ASSERT(tab->type == NK_CONSOLE_TAB);
    NK_ASSERT(tab->parent != NULL);
    return nk_console_tabs_get_selected_tab(tab->parent) == tab;
}

NK_API void nk_console_tabs_set_selected(nk_console* tab) {
    NK_ASSERT(tab != NULL);
    NK_ASSERT(tab->type == NK_CONSOLE_TAB);
    NK_ASSERT(tab->parent != NULL);
    nk_console_tabs_switch(tab->parent, tab);
}

NK_API int nk_console_tabs_selected_index(nk_console* tabs) {
    nk_console* selected = nk_console_tabs_get_selected_tab(tabs);
    return selected != NULL ? nk_console_get_widget_index(selected) : -1;
}

NK_API void nk_console_tabs_set_selected_index(nk_console* tabs, int index) {
    NK_ASSERT(tabs != NULL);
    NK_ASSERT(tabs->data != NULL);
    NK_ASSERT(tabs->type == NK_CONSOLE_TABS);
    nk_console_tabs_switch(tabs, nk_console_tabs_get_tab(tabs, index));
}

NK_API nk_console* nk_console_tab(nk_console* tabs, const char* label) {
    NK_ASSERT(tabs != NULL);
    NK_ASSERT(tabs->type == NK_CONSOLE_TABS);

    nk_console* tab = nk_console_label(tabs, label);
    if (tab == NULL) {
        return NULL;
    }
    tab->type = NK_CONSOLE_TAB;
    tab->render = NULL;
    nk_console_add_event(tab, NK_CONSOLE_EVENT_DESTROYED, &nk_console_tab_event_destroyed);

    // The first tab starts out selected.
    nk_console_tabs_data* data = (nk_console_tabs_data*)tabs->data;
    if (data->selected == NULL) {
        data->selected = tab;
    }

    return tab;
}

NK_API nk_console* nk_console_tabs(nk_console* parent) {
    NK_ASSERT(parent != NULL);

    nk_console_tabs_data* data = (nk_console_tabs_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_tabs_data));
    if (data == NULL) {
        return NULL;
    }
    nk_zero(data, sizeof(nk_console_tabs_data));

    nk_console* tabs = nk_console_label(parent, NULL);
    if (tabs == NULL) {
        NK_CONSOLE_FREE(nk_handle_id(0), data);
        return NULL;
    }
    tabs->type = NK_CONSOLE_TABS;
    tabs->data = (void*)data;
    tabs->selectable = nk_true;
    tabs->render = nk_console_tabs_render;

    return tabs;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TABS_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
