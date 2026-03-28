#ifndef NK_CONSOLE_LIST_VIEW_H__
#define NK_CONSOLE_LIST_VIEW_H__

/**
 * Callback that will be used to retrieve the label of a given list view item.
 *
 * @param list_view The list view widget.
 * @param index Which row to get the label for.
 * @return The label for the item.
 *
 * @see nk_console_list_view_data
 */
typedef const char* (*nk_console_list_view_get_label)(struct nk_console* list_view, nk_uint index);

/**
 * Data associated with a List View Widget.
 */
typedef struct nk_console_list_view_data {
    struct nk_list_view view; /** The Nuklear list view object itself. */
    nk_flags flags; /** Flags that are used for the list_view. */
    nk_uint row_count; /** The number of available rows in the dataset. */
    nk_uint rows_visible; /** The number of rows to display when the user is looking at the list. */
    nk_uint scroll_pointer; /** Tracks where the user is scrolling the list. */
    nk_uint selected; /** Which item has been selected. */
    nk_uint _scroll_y; /** Helps calculate the list view scroll. */
    nk_console_list_view_get_label get_label_callback; /** The callback used to retrieve the labels for each item. */
} nk_console_list_view_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Creates a List View that can display thousands of items.
 *
 * @param parent Where the element should be placed.
 * @param id A unique identifier for the list view.
 * @param rows_visible The number of rows to be shown at any given time.
 * @param item_count The number of items in the data set.
 * @param get_label_callback The callback that will be used to retrieve individual labels.
 *
 * @see nk_console_list_view_get_label
 */
NK_API nk_console* nk_console_list_view(nk_console* parent, const char* id, int rows_visible, nk_uint item_count, nk_console_list_view_get_label get_label_callback);

/**
 * Get the currently selected index.
 *
 * @param list_view The List View widget.
 * @return The list view item that the user currently has selected.
 */
NK_API nk_uint nk_console_list_view_selected(nk_console* list_view);

/**
 * Get the currently selected label.
 *
 * @param list_view The List View widget.
 * @return The label associated with the currently selected item.
 */
NK_API const char* nk_console_list_view_selected_label(nk_console* list_view);

/**
 * Retrieve that height of an individual row in the list view.
 *
 * @param list_view The List View widget.
 * @return The height of an individual row.
 */
NK_API float nk_console_list_view_row_height(nk_console* list_view);

/**
 * Retrieve the flags for the given List View Widget.
 */
NK_API nk_flags nk_console_list_view_flags(nk_console* list_view);

/**
 * Set the flags for the given List View Widget.
 *
 * For example: NK_WINDOW_BORDER, NK_WINDOW_TITLE
 */
NK_API void nk_console_list_view_set_flags(nk_console* list_view, nk_flags flags);

NK_API void nk_console_list_view_set_item_count(nk_console* list_view, nk_uint item_count);
NK_API nk_uint nk_console_list_view_item_count(nk_console* list_view);

#if defined(__cplusplus)
}
#endif

#endif /* NK_CONSOLE_LIST_VIEW_H__ */

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_LIST_VIEW_IMPLEMENTATION_ONCE
#define NK_CONSOLE_LIST_VIEW_IMPLEMENTATION_ONCE

NK_API nk_uint nk_console_list_view_item_count(nk_console* list_view) {
    if (list_view == NULL || list_view->data == NULL || list_view->type != NK_CONSOLE_LIST_VIEW) {
        return 0;
    }
    nk_console_list_view_data* data = (nk_console_list_view_data*)list_view->data;
    return data->row_count;
}

NK_API void nk_console_list_view_set_item_count(nk_console* list_view, nk_uint item_count) {
    if (list_view == NULL || list_view->data == NULL || list_view->type != NK_CONSOLE_LIST_VIEW) {
        return;
    }
    nk_console_list_view_data* data = (nk_console_list_view_data*)list_view->data;
    data->row_count = item_count;
    data->selected = 0;
    data->_scroll_y = 0;
    if (data->view.scroll_pointer) {
        *data->view.scroll_pointer = 0;
    }
}

NK_API nk_flags nk_console_list_view_flags(nk_console* list_view) {
    if (list_view == NULL || list_view->data == NULL || list_view->type != NK_CONSOLE_LIST_VIEW) {
        return 0;
    }
    nk_console_list_view_data* data = (nk_console_list_view_data*)list_view->data;
    return data->flags;
}

NK_API void nk_console_list_view_set_flags(nk_console* list_view, nk_flags flags) {
    if (list_view == NULL || list_view->data == NULL || list_view->type != NK_CONSOLE_LIST_VIEW) {
        return;
    }
    nk_console_list_view_data* data = (nk_console_list_view_data*)list_view->data;
    data->flags = flags;
}

NK_API float nk_console_list_view_row_height(nk_console* list_view) {
    if (list_view == NULL) {
        return 0;
    }
    return list_view->ctx->style.font->height + list_view->ctx->style.button.border * 2.0f + list_view->ctx->style.button.padding.y * 2.0f;
}

NK_API nk_uint nk_console_list_view_selected(nk_console* list_view) {
    if (!list_view || !list_view->data || list_view->type != NK_CONSOLE_LIST_VIEW)
        return 0;
    nk_console_list_view_data* data = (nk_console_list_view_data*)list_view->data;
    return data->selected;
}

NK_API const char* nk_console_list_view_selected_label(nk_console* list_view) {
    if (!list_view || !list_view->data || list_view->type != NK_CONSOLE_LIST_VIEW)
        return NULL;
    nk_console_list_view_data* data = (nk_console_list_view_data*)list_view->data;
    if (data->selected >= (nk_uint)data->row_count)
        return NULL;
    return data->get_label_callback(list_view, data->selected);
}

NK_API struct nk_rect nk_console_list_view_render(nk_console* widget) {
    if (widget == NULL || widget->data == NULL || widget->type != NK_CONSOLE_LIST_VIEW) {
        return nk_rect(0, 0, 0, 0);
    }
    nk_console_list_view_data* data = (nk_console_list_view_data*)widget->data;

    nk_console* top = nk_console_get_top(widget);
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    nk_bool is_active = nk_console_is_active_widget(widget);
    nk_uint original_selected = data->selected;

    float row_height = nk_console_list_view_row_height(top);
    float scroll_row_height = row_height + top->ctx->style.window.spacing.y;

    // Layout the widget with the correct visible height so widget_bounds is accurate.
    struct nk_rect widget_bounds = nk_layout_widget_bounds(top->ctx);
    widget_bounds.h = row_height * (float)data->rows_visible;

    if (widget->disabled) {
        nk_widget_disable_begin(top->ctx);
    }

    // Handle keyboard/gamepad navigation.
    if (is_active && !top_data->input_processed) {
        // Hold-to-accelerate timers — mirrors nk_console_check_up_down.
        nk_bool up_held   = nk_console_button_down(top, NK_GAMEPAD_BUTTON_UP);
        nk_bool down_held = nk_console_button_down(top, NK_GAMEPAD_BUTTON_DOWN);
        nk_bool repeat_fire = nk_false;
        if (up_held || down_held) {
            if (top->ctx->delta_time_seconds > 0) {
                top_data->up_down_hold_timer  += top->ctx->delta_time_seconds;
                top_data->up_down_repeat_timer += top->ctx->delta_time_seconds;
                float interval = NK_MAX(0.045f,
                    0.5f - top_data->up_down_hold_timer * 0.125f);
                if (top_data->up_down_repeat_timer >= interval) {
                    repeat_fire = nk_true;
                    top_data->up_down_repeat_timer -= interval;
                }
            }
        } else {
            top_data->up_down_hold_timer  = 0;
            top_data->up_down_repeat_timer = 0;
        }

        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LB)) {
            // Page up: jump selection up by rows_visible items.
            if (data->selected > 0) {
                data->selected = NK_MAX(0, data->selected - data->rows_visible);
                if (data->view.scroll_pointer) {
                    nk_uint new_scroll = (nk_uint)data->selected * (nk_uint)scroll_row_height;
                    *data->view.scroll_pointer = new_scroll;
                    data->_scroll_y = new_scroll;
                }
            }
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RB)) {
            // Page down: jump selection down by rows_visible items.
            if (data->row_count > 0 && data->selected < data->row_count - 1) {
                data->selected = NK_MIN(data->row_count - 1, data->selected + data->rows_visible);
                if (data->view.scroll_pointer && data->view.count > 0) {
                    int last_full = data->view.begin + data->view.count - 2;
                    if ((int)data->selected > last_full) {
                        int new_begin = (int)data->selected - (data->view.count - 2);
                        nk_uint max_scroll = (nk_uint)((float)(NK_MAX(0, data->row_count - (nk_uint)data->view.count)) * scroll_row_height);
                        nk_uint new_scroll = (nk_uint)((float)NK_MAX(0, new_begin) * scroll_row_height);
                        if (new_scroll > max_scroll) new_scroll = max_scroll;
                        *data->view.scroll_pointer = new_scroll;
                        data->_scroll_y = new_scroll;
                    }
                }
            }
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_UP) ||
                (up_held && repeat_fire)) {
            if (data->selected > 0) {
                data->selected--;
                if (data->view.scroll_pointer && (int)data->selected < data->view.begin) {
                    nk_uint new_scroll = (nk_uint)data->selected * (nk_uint)scroll_row_height;
                    *data->view.scroll_pointer = new_scroll;
                    data->_scroll_y = new_scroll;
                }
            }
            else {
                // At top: move focus to the previous sibling widget.
                int idx = nk_console_get_widget_index(widget);
                while (--idx >= 0) {
                    nk_console* t = widget->parent->children[idx];
                    if (nk_console_selectable(t)) {
                        nk_console_set_active_widget(t);
                        top_data->scroll_to_widget = t;
                        break;
                    }
                }
            }
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_DOWN) ||
                (down_held && repeat_fire)) {
            if (data->row_count > 0 && data->selected < data->row_count - 1) {
                data->selected++;
                if (data->view.scroll_pointer && data->view.count > 0) {
                    // Scroll if selected is at or past the last fully-visible row.
                    int last_full = data->view.begin + data->view.count - 2;
                    if ((int)data->selected > last_full) {
                        int new_begin = (int)data->selected - (data->view.count - 2);
                        nk_uint max_scroll = (nk_uint)NK_MAX(0, data->row_count - (nk_uint)data->view.count) * (nk_uint)scroll_row_height;
                        nk_uint new_scroll = (nk_uint)((float)NK_MAX(0, new_begin) * scroll_row_height);
                        if (new_scroll > max_scroll) new_scroll = max_scroll;
                        if (new_scroll != data->_scroll_y) {
                            *data->view.scroll_pointer = new_scroll;
                            data->_scroll_y = new_scroll;
                        }
                    }
                }
            }
            else {
                // At bottom: move focus to the next sibling widget.
                int idx = nk_console_get_widget_index(widget);
                int sibling_count = (int)cvector_size(widget->parent->children);
                while (++idx < sibling_count) {
                    nk_console* t = widget->parent->children[idx];
                    if (nk_console_selectable(t)) {
                        nk_console_set_active_widget(t);
                        top_data->scroll_to_widget = t;
                        break;
                    }
                }
            }
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT)) {
            // Move focus to the previous sibling widget.
            int idx = nk_console_get_widget_index(widget);
            while (--idx >= 0) {
                nk_console* t = widget->parent->children[idx];
                if (nk_console_selectable(t)) {
                    nk_console_set_active_widget(t);
                    top_data->scroll_to_widget = t;
                    break;
                }
            }
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT)) {
            // Move focus to the next sibling widget.
            int idx = nk_console_get_widget_index(widget);
            int sibling_count = (int)cvector_size(widget->parent->children);
            while (++idx < sibling_count) {
                nk_console* t = widget->parent->children[idx];
                if (nk_console_selectable(t)) {
                    nk_console_set_active_widget(t);
                    top_data->scroll_to_widget = t;
                    break;
                }
            }
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_A) && !widget->disabled) {
            nk_console_trigger_event(widget, NK_CONSOLE_EVENT_CLICKED);
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_B)) {
            if (widget->parent != NULL) {
                nk_console_navigate_back(widget->parent);
            }
            top_data->input_processed = nk_true;
        }
    }

    // Display the visible rows.
    nk_layout_row_dynamic(top->ctx, widget_bounds.h, 1);
    if (nk_list_view_begin(top->ctx, &data->view, widget->label, data->flags, (int)row_height, (int)data->row_count)) {
        // Cache the normal button style to restore it later on.
        struct nk_style_item saved_normal = top->ctx->style.button.normal;
        struct nk_color saved_text = top->ctx->style.button.text_normal;

        nk_layout_row_dynamic(top->ctx, row_height, 1);
        for (int i = 0; i < data->view.count; ++i) {
            const char* label = data->get_label_callback(widget, (nk_uint)(data->view.begin + i));
            if (label == NULL) {
                // TODO: When the list view item doesn't have a label, figure out a way to continue without misaligning selected.
                break;
            }

            // Mouse Selection
            if (nk_input_is_mouse_moved(&top->ctx->input) && nk_widget_is_hovered(top->ctx) && !widget->disabled) {
                data->selected = (nk_uint)(data->view.begin + i);
                nk_console_set_active_widget(widget);
            }

            // Render each row as a selectable button, highlighting if selected.
            nk_bool is_selected = (nk_uint)(data->view.begin + i) == data->selected;

            // Active items should get the active button style.
            if (is_selected && is_active) {
                top->ctx->style.button.normal = top->ctx->style.button.hover;
                top->ctx->style.button.text_normal = top->ctx->style.button.text_hover;
            }

            nk_bool mouse_clicked = nk_button_label(top->ctx, label);

            // Restore the original button style.
            if (is_selected && is_active) {
                top->ctx->style.button.normal      = saved_normal;
                top->ctx->style.button.text_normal = saved_text;
            }

            if (mouse_clicked && top_data->input_processed == nk_false) {
                data->selected = (nk_uint)(data->view.begin + i);
                top_data->input_processed = nk_true;
                nk_console_trigger_event(widget, NK_CONSOLE_EVENT_CLICKED);
            }
        }
        nk_list_view_end(&data->view);

        // Update the scroll location.
        if (data->view.scroll_pointer) {
            data->_scroll_y = *data->view.scroll_pointer;
        }
    }

    if (widget->disabled) {
        nk_widget_disable_end(top->ctx);
    }

    // Trigger the changed event if they've selected a different element.
    if (original_selected != data->selected) {
        nk_console_trigger_event(widget, NK_CONSOLE_EVENT_CHANGED);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_list_view(nk_console* parent, const char* id, int rows_visible, nk_uint item_count, nk_console_list_view_get_label get_label_callback) {
    if (parent == NULL || get_label_callback == NULL) {
        return NULL;
    }

    nk_console_list_view_data* data = (nk_console_list_view_data*)nk_console_malloc(nk_handle_ptr(NULL), NULL, sizeof(nk_console_list_view_data));
    if (data == NULL) {
        return NULL;
    }
    nk_zero(data, sizeof(nk_console_list_view_data));
    data->row_count = item_count;
    data->rows_visible = rows_visible <= 0 ? (nk_uint)10 : (nk_uint)rows_visible;
    data->get_label_callback = get_label_callback;
    data->flags = NK_WINDOW_BORDER;

    nk_console* widget = nk_console_label(parent, id);
    if (widget == NULL) {
        NK_CONSOLE_FREE(nk_handle_id(0), data);
        return NULL;
    }

    widget->selectable = nk_true;
    widget->type = NK_CONSOLE_LIST_VIEW;
    widget->render = &nk_console_list_view_render;
    widget->data = data;

    return widget;
}

#endif /* NK_CONSOLE_LIST_VIEW_IMPLEMENTATION_ONCE */
#endif /* NK_CONSOLE_IMPLEMENTATION && !NK_CONSOLE_HEADER_ONLY */
