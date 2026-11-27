#ifndef NK_CONSOLE_TABS_H__
#define NK_CONSOLE_TABS_H__

typedef struct nk_console_tabs_data {
    int selected; /** The index of the selected tab. */
} nk_console_tabs_data;

typedef struct nk_console_tab_data {
    struct nk_console** referenced_children; /** The widgets this tab shows, which are siblings of the tabs widget. */
    struct nk_console* last_active; /** The widget that was last active in the tab, restored when switching back to it. */
} nk_console_tab_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Create a Tabs widget, which displays a row of tab headers along with the widgets of the selected tab.
 *
 * Switch between tabs with LB/RB (Shift+Up/Down on keyboard), with Left/Right when the tab headers are focused, or by clicking a header.
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
 * Get the index of the selected tab.
 *
 * @param tabs The Tabs widget.
 *
 * @return The index of the selected tab.
 */
NK_API int nk_console_tabs_selected(nk_console* tabs);

/**
 * Select the tab at the given index.
 *
 * @param tabs The Tabs widget.
 * @param index The index of the tab to select.
 */
NK_API void nk_console_tabs_set_selected(nk_console* tabs, int index);

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
 * Whether or not the given widget is shown by the tab.
 */
static nk_bool nk_console_tab_references(nk_console* tab, nk_console* widget) {
    nk_console_tab_data* data = (nk_console_tab_data*)tab->data;
    for (size_t i = 0; i < cvector_size(data->referenced_children); i++) {
        if (data->referenced_children[i] == widget) {
            return nk_true;
        }
    }
    return nk_false;
}

/**
 * Show the selected tab's widgets, and hide all the others.
 */
static void nk_console_tabs_apply_selected(nk_console* tabs) {
    nk_console_tabs_data* data = (nk_console_tabs_data*)tabs->data;
    for (int i = 0; i < (int)cvector_size(tabs->children); i++) {
        nk_console* tab = nk_console_tabs_get_tab(tabs, i);
        if (tab == NULL) {
            continue;
        }
        nk_console_tab_data* tab_data = (nk_console_tab_data*)tab->data;
        for (size_t j = 0; j < cvector_size(tab_data->referenced_children); j++) {
            tab_data->referenced_children[j]->visible = (i == data->selected) ? nk_true : nk_false;
        }
    }

    // Trees within the selected tab only show their children when expanded.
    nk_console* selected = nk_console_tabs_get_tab(tabs, data->selected);
    if (selected != NULL) {
        nk_console_tab_data* tab_data = (nk_console_tab_data*)selected->data;
        for (size_t i = 0; i < cvector_size(tab_data->referenced_children); i++) {
            nk_console* tree = tab_data->referenced_children[i];
            if (tree->type != NK_CONSOLE_TREE || tree->data == NULL) {
                continue;
            }
            nk_console_tree_data* tree_data = (nk_console_tree_data*)tree->data;
            for (size_t j = 0; j < cvector_size(tree_data->referenced_children); j++) {
                if (tree_data->referenced_children[j] != NULL) {
                    tree_data->referenced_children[j]->visible = tree->visible && nk_console_tree_expanded(tree);
                }
            }
        }
    }

    // Force the scrollbar to update if needed.
    nk_console* top = nk_console_get_top(tabs);
    if (top != NULL && top->data != NULL) {
        ((nk_console_top_data*)top->data)->scrollbar_required = nk_true;
    }
}

/**
 * Switch to the tab at the given index. When focus was within the previous tab, it moves to the new one.
 */
static void nk_console_tabs_switch(nk_console* tabs, int index) {
    nk_console_tabs_data* data = (nk_console_tabs_data*)tabs->data;
    nk_console* tab = nk_console_tabs_get_tab(tabs, index);
    if (tab == NULL || index == data->selected) {
        return;
    }

    // Remember which widget was active in the previous tab.
    nk_console* screen = tabs->parent;
    nk_console* active = screen != NULL ? screen->activeWidget : NULL;
    nk_console* previous = nk_console_tabs_get_tab(tabs, data->selected);
    nk_bool focus_within = nk_false;
    if (previous != NULL && active != NULL && nk_console_tab_references(previous, active)) {
        ((nk_console_tab_data*)previous->data)->last_active = active;
        focus_within = nk_true;
    }

    data->selected = index;
    nk_console_tabs_apply_selected(tabs);

    // Move focus into the new tab, preferring the widget that was last active there.
    if (focus_within) {
        nk_console_tab_data* tab_data = (nk_console_tab_data*)tab->data;
        nk_console* target = tab_data->last_active;
        if (target == NULL || !nk_console_selectable(target)) {
            target = NULL;
            for (size_t i = 0; i < cvector_size(tab_data->referenced_children) && target == NULL; i++) {
                if (nk_console_selectable(tab_data->referenced_children[i])) {
                    target = tab_data->referenced_children[i];
                }
            }
        }
        if (target == NULL) {
            target = tabs;
        }
        nk_console_set_active_widget(target);
        nk_console* top = nk_console_get_top(tabs);
        ((nk_console_top_data*)top->data)->scroll_to_widget = target;
    }

    nk_console_trigger_event(tabs, NK_CONSOLE_EVENT_CHANGED);
}

/**
 * Select the next or previous enabled tab, wrapping around the ends.
 *
 * @return True if the selected tab changed.
 */
static nk_bool nk_console_tabs_step(nk_console* tabs, int direction) {
    nk_console_tabs_data* data = (nk_console_tabs_data*)tabs->data;
    int count = (int)cvector_size(tabs->children);
    int index = data->selected;
    for (int i = 0; i < count; i++) {
        index = (index + direction + count) % count;
        nk_console* tab = nk_console_tabs_get_tab(tabs, index);
        if (tab != NULL && tab->visible && !tab->disabled) {
            if (index == data->selected) {
                return nk_false;
            }
            nk_console_tabs_switch(tabs, index);
            return nk_true;
        }
    }
    return nk_false;
}

/**
 * Find the first visible Tabs widget within the given parent.
 */
static nk_console* nk_console_tabs_find(nk_console* parent) {
    for (size_t i = 0; i < cvector_size(parent->children); i++) {
        nk_console* child = parent->children[i];
        if (child != NULL && child->type == NK_CONSOLE_TABS && child->visible) {
            return child;
        }
    }
    return NULL;
}

/**
 * Find the tab that shows the given widget, or NULL if it isn't within a tab.
 */
static nk_console* nk_console_tabs_find_tab_of(nk_console* widget) {
    nk_console* parent = widget->parent;
    if (parent == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < cvector_size(parent->children); i++) {
        nk_console* tabs = parent->children[i];
        if (tabs == NULL || tabs->type != NK_CONSOLE_TABS) {
            continue;
        }
        for (int j = 0; j < (int)cvector_size(tabs->children); j++) {
            nk_console* tab = nk_console_tabs_get_tab(tabs, j);
            if (tab != NULL && nk_console_tab_references(tab, widget)) {
                return tab;
            }
        }
    }
    return NULL;
}

/**
 * Forget a widget the tab references when the widget is freed.
 */
static void nk_console_tab_child_destroyed(nk_console* child, void* user_data) {
    nk_console* tab = (nk_console*)user_data;
    nk_console_tab_data* data = (nk_console_tab_data*)tab->data;
    for (size_t i = 0; i < cvector_size(data->referenced_children); i++) {
        if (data->referenced_children[i] == child) {
            cvector_erase(data->referenced_children, i);
            break;
        }
    }
    if (data->last_active == child) {
        data->last_active = NULL;
    }
}

/**
 * Have the tab show the given widget, which is a sibling of the tabs widget.
 */
static void nk_console_tab_reference(nk_console* tab, nk_console* child) {
    nk_console_tab_data* data = (nk_console_tab_data*)tab->data;
    cvector_push_back(data->referenced_children, child);
    nk_console_add_event_handler(child, NK_CONSOLE_EVENT_DESTROYED, &nk_console_tab_child_destroyed, tab, NULL);

    // Only show the widget when its tab is selected.
    nk_console* tabs = tab->parent;
    if (nk_console_tabs_get_tab(tabs, nk_console_tabs_selected(tabs)) != tab) {
        child->visible = nk_false;
    }
}

/**
 * Get the number of widgets referenced by all tabs, which sit right after the tabs widget.
 */
static size_t nk_console_tabs_referenced_count(nk_console* tabs) {
    size_t count = 0;
    for (int i = 0; i < (int)cvector_size(tabs->children); i++) {
        nk_console* tab = nk_console_tabs_get_tab(tabs, i);
        if (tab != NULL) {
            count += cvector_size(((nk_console_tab_data*)tab->data)->referenced_children);
        }
    }
    return count;
}

static void nk_console_tab_event_destroyed(nk_console* tab, void* user_data) {
    NK_UNUSED(user_data);
    nk_console_tab_data* data = (nk_console_tab_data*)tab->data;
    if (data == NULL) {
        return;
    }

    // Free all the referenced children, removing them from the parent. Freeing a tree also frees its
    // children, which removes them from the list, so always take the first.
    while (cvector_size(data->referenced_children) > 0) {
        nk_console* child = data->referenced_children[0];
        cvector_erase(data->referenced_children, 0);

        // The tab is going away, so stop listening for the child being destroyed.
        for (size_t i = 0; i < cvector_size(child->events); i++) {
            if (child->events[i].callback == &nk_console_tab_child_destroyed && child->events[i].user_data == tab) {
                child->events[i].callback = NULL;
            }
        }

        // When the parent is freeing all its children, they're already detached from it, and the parent frees them itself.
        if (nk_console_get_widget_index(child) >= 0) {
            nk_console_free(child);
        }
    }

    cvector_free(data->referenced_children);
    data->referenced_children = NULL;
}

static struct nk_rect nk_console_tabs_render(nk_console* tabs) {
    NK_ASSERT(tabs != NULL);
    NK_ASSERT(tabs->data != NULL);
    nk_console_tabs_data* data = (nk_console_tabs_data*)tabs->data;
    struct nk_context* ctx = tabs->ctx;

    // Make sure a valid tab is selected, in case the selected one was freed.
    if (nk_console_tabs_get_tab(tabs, data->selected) == NULL) {
        for (int i = 0; i < (int)cvector_size(tabs->children); i++) {
            if (nk_console_tabs_get_tab(tabs, i) != NULL) {
                data->selected = i;
                nk_console_tabs_apply_selected(tabs);
                break;
            }
        }
    }

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

    // Highlight the selected tab when the tab headers are focused.
    nk_bool active = nk_console_is_active_widget(tabs);
    struct nk_style_item normal_active = ctx->style.selectable.normal_active;
    struct nk_color text_normal_active = ctx->style.selectable.text_normal_active;
    if (active) {
        ctx->style.selectable.normal_active = ctx->style.selectable.hover_active;
        ctx->style.selectable.text_normal_active = ctx->style.selectable.text_hover_active;
    }

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
        nk_bool selected = (i == data->selected) ? nk_true : nk_false;
        const char* label = tab->label != NULL ? tab->label : "";
        nk_bool clicked = tab->label_length > 0 ? nk_selectable_text(ctx, label, tab->label_length, NK_TEXT_CENTERED, &selected) : nk_selectable_label(ctx, label, NK_TEXT_CENTERED, &selected);
        if (disabled) {
            nk_widget_disable_end(ctx);
        }
        else if (clicked) {
            nk_console_tabs_switch(tabs, i);
        }
    }

    ctx->style.selectable.normal_active = normal_active;
    ctx->style.selectable.text_normal_active = text_normal_active;

    // Left/Right switch tabs while the tab headers are focused.
    if (active && !tabs->disabled) {
        nk_console* top = nk_console_get_top(tabs);
        nk_console_top_data* top_data = (nk_console_top_data*)top->data;
        if (!top_data->input_processed) {
            if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT)) {
                nk_console_tabs_step(tabs, -1);
                top_data->input_processed = nk_true;
            }
            else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT)) {
                nk_console_tabs_step(tabs, 1);
                top_data->input_processed = nk_true;
            }
        }
        nk_console_check_up_down(tabs);
    }

    return widget_bounds;
}

NK_API int nk_console_tabs_selected(nk_console* tabs) {
    NK_ASSERT(tabs != NULL);
    NK_ASSERT(tabs->data != NULL);
    NK_ASSERT(tabs->type == NK_CONSOLE_TABS);
    return ((nk_console_tabs_data*)tabs->data)->selected;
}

NK_API void nk_console_tabs_set_selected(nk_console* tabs, int index) {
    NK_ASSERT(tabs != NULL);
    NK_ASSERT(tabs->data != NULL);
    NK_ASSERT(tabs->type == NK_CONSOLE_TABS);
    nk_console_tabs_switch(tabs, index);
}

NK_API nk_console* nk_console_tab(nk_console* tabs, const char* label) {
    NK_ASSERT(tabs != NULL);
    NK_ASSERT(tabs->type == NK_CONSOLE_TABS);

    nk_console_tab_data* data = (nk_console_tab_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_tab_data));
    if (data == NULL) {
        return NULL;
    }
    nk_zero(data, sizeof(nk_console_tab_data));

    nk_console* tab = nk_console_label(tabs, label);
    if (tab == NULL) {
        NK_CONSOLE_FREE(nk_handle_id(0), data);
        return NULL;
    }
    tab->type = NK_CONSOLE_TAB;
    tab->data = (void*)data;
    tab->render = NULL;
    nk_console_add_event(tab, NK_CONSOLE_EVENT_DESTROYED, &nk_console_tab_event_destroyed);

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
