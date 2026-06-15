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

#ifndef NK_CONSOLE_LIST_VIEW_SEARCH_BUFFER_SIZE
#define NK_CONSOLE_LIST_VIEW_SEARCH_BUFFER_SIZE 64
#endif

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
    nk_bool searchable; /** When true, a sibling search field filters visible items. */
    char search_buffer[NK_CONSOLE_LIST_VIEW_SEARCH_BUFFER_SIZE]; /** Search filter text. */
    struct nk_console* search; /** The sibling search textedit widget, if one was added. */
} nk_console_list_view_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Creates a List View that can display thousands of items.
 *
 * @param parent Where the element should be placed.
 * @param id A unique identifier for the list view.
 * @param rows_visible The number of rows to be shown at any given time. Pass 0 to fill the remaining window height.
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

/**
 * Enable or disable an optional search/filter textedit above the list view.
 *
 * When enabled, a textedit widget is inserted as a sibling directly before this
 * list view. Typing in that field case-insensitively filters the displayed rows
 * (ASCII case folding only). When disabled, that field is removed and the filter
 * is cleared. The value returned by nk_console_list_view_selected() is always a
 * real (unfiltered) item index; nk_console_list_view_selected_label() returns NULL
 * when an active filter matches nothing.
 *
 * @param list_view The List View widget.
 * @param searchable nk_true to add the search field, nk_false to remove it.
 */
NK_API void nk_console_list_view_set_searchable(nk_console* list_view, nk_bool searchable);

/** Programmatically select an item and scroll it into view.
 *
 * @param list_view The List View widget.
 * @param index The zero-based index to select. Clamped to [0, row_count).
 */
NK_API void nk_console_list_view_set_selected(nk_console* list_view, nk_uint index);

#if defined(__cplusplus)
}
#endif

#endif /* NK_CONSOLE_LIST_VIEW_H__ */

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_LIST_VIEW_IMPLEMENTATION_ONCE
#define NK_CONSOLE_LIST_VIEW_IMPLEMENTATION_ONCE

/**
 * ASCII case fold: lowercases A-Z only. Other bytes (digits, symbols, UTF-8
 * continuation bytes) are returned unchanged so they don't alias one another
 * the way a blanket `c | 32` would (e.g. '[' vs '{', '@' vs '`').
 */
static int nk_console_list_view_tolower(int c) {
    return (c >= 'A' && c <= 'Z') ? (c + 32) : c;
}

/**
 * Case-insensitive ASCII contains check. Returns nk_true if label contains filter.
 *
 * Folding is ASCII-only; non-ASCII (UTF-8) bytes are compared verbatim.
 */
static nk_bool nk_console_list_view_item_matches(const char* label, const char* filter) {
    if (filter == NULL || filter[0] == '\0') return nk_true;
    if (label == NULL) return nk_false;
    for (int i = 0; label[i] != '\0'; i++) {
        int j = 0;
        while (filter[j] != '\0' && label[i + j] != '\0' &&
               nk_console_list_view_tolower((unsigned char)label[i + j]) ==
               nk_console_list_view_tolower((unsigned char)filter[j])) {
            j++;
        }
        if (filter[j] == '\0') return nk_true;
    }
    return nk_false;
}

/**
 * Returns the display (filtered) index of real_idx. O(real_idx).
 */
static nk_uint nk_console_list_view_display_index(nk_console* widget, nk_console_list_view_data* data, nk_uint real_idx, const char* filter) {
    if (filter == NULL || filter[0] == '\0') return real_idx;
    nk_uint disp = 0;
    for (nk_uint k = 0; k < real_idx && k < data->row_count; k++) {
        const char* lbl = data->get_label_callback(widget, k);
        if (nk_console_list_view_item_matches(lbl, filter)) disp++;
    }
    return disp;
}

/**
 * Returns the real index of the disp_idx-th matching item.
 */
static nk_uint nk_console_list_view_nth_match(nk_console* widget, nk_console_list_view_data* data, nk_uint disp_idx, const char* filter) {
    if (filter == NULL || filter[0] == '\0') return disp_idx;
    nk_uint count = 0;
    for (nk_uint k = 0; k < data->row_count; k++) {
        const char* lbl = data->get_label_callback(widget, k);
        if (nk_console_list_view_item_matches(lbl, filter)) {
            if (count == disp_idx) return k;
            count++;
        }
    }
    return data->row_count > 0 ? data->row_count - 1 : 0;
}

/**
 * Apply a scroll offset to both the live scroll pointer and the cached value.
 */
static void nk_console_list_view_apply_scroll(nk_console_list_view_data* data, nk_uint scroll) {
    if (data->view.scroll_pointer) {
        *data->view.scroll_pointer = scroll;
        data->_scroll_y = scroll;
    }
}

/**
 * Move focus to the nearest selectable sibling in the given direction.
 *
 * dir == -1 for the previous sibling, dir == +1 for the next
 */
static void nk_console_list_view_focus_sibling(nk_console* widget, nk_console_top_data* top_data, int dir) {
    int count = (int)cvector_size(widget->parent->children);
    int idx = nk_console_get_widget_index(widget);
    for (idx += dir; idx >= 0 && idx < count; idx += dir) {
        nk_console* t = widget->parent->children[idx];
        if (nk_console_selectable(t)) {
            nk_console_set_active_widget(t);
            top_data->scroll_to_widget = t;
            break;
        }
    }
}

/**
 * Scroll so the row at display index sel_disp is visible near the bottom edge,
 * used when navigating downward (one row or a page at a time).
 */
static void nk_console_list_view_scroll_into_view_down(nk_console_list_view_data* data, nk_uint sel_disp, nk_uint display_count, float scroll_row_height) {
    if (!data->view.scroll_pointer || data->view.count <= 0) {
        return;
    }
    int last_full = data->view.begin + data->view.count - 2;
    if ((int)sel_disp <= last_full) {
        return;
    }
    int new_begin = (int)sel_disp - (data->view.count - 2);
    nk_uint max_scroll = (nk_uint)((float)NK_MAX(0, (int)display_count - (int)data->view.count) * scroll_row_height);
    nk_uint new_scroll = (nk_uint)((float)NK_MAX(0, new_begin) * scroll_row_height);
    if (new_scroll > max_scroll) {
        new_scroll = max_scroll;
    }
    nk_console_list_view_apply_scroll(data, new_scroll);
}

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
    float height = list_view->ctx->style.font->height + list_view->ctx->style.button.border * 2.0f + list_view->ctx->style.button.padding.y * 2.0f;
    // Round up to a whole pixel. nk_list_view_begin() casts the row height to int
    // for its visible-row count, while the rows themselves are laid out at this
    // (float) height; keeping it integral makes the two agree so the count, scroll
    // math, and drawn rows stay in lockstep when the font height is fractional.
    return (float)(int)(height + 0.9999f);
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
    const char* label = data->get_label_callback(list_view, data->selected);
    // When a filter is active and the selected item doesn't pass it (i.e. there
    // are no matches), there is no meaningful visible selection.
    if (data->searchable && data->search_buffer[0] != '\0' &&
            !nk_console_list_view_item_matches(label, data->search_buffer)) {
        return NULL;
    }
    return label;
}

NK_API void nk_console_list_view_set_selected(nk_console* list_view, nk_uint index) {
    if (!list_view || !list_view->data || list_view->type != NK_CONSOLE_LIST_VIEW) return;
    nk_console_list_view_data* data = (nk_console_list_view_data*)list_view->data;
    if (data->row_count == 0) return;
    if (index >= data->row_count) index = data->row_count - 1;
    data->selected = index;

    // Scroll so the selected item is at the top of the visible area.
    float row_height = nk_console_list_view_row_height(list_view);
    // Truncate spacing to int to mirror nk_list_view_begin's (int)item_spacing.y,
    // so this stride matches Nuklear's scroll-to-row math exactly.
    float scroll_row_height = row_height + (float)(int)list_view->ctx->style.window.spacing.y;
    nk_uint new_scroll = (nk_uint)((float)index * scroll_row_height);
    data->_scroll_y = new_scroll;
    if (data->view.scroll_pointer) {
        *data->view.scroll_pointer = new_scroll;
    }
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

    // Resolve active filter string.
    const char* filter = (data->searchable && data->search_buffer[0] != '\0') ? data->search_buffer : NULL;

    // Count how many items pass the current filter.
    nk_uint display_count = data->row_count;
    if (filter != NULL) {
        display_count = 0;
        for (nk_uint k = 0; k < data->row_count; k++) {
            const char* lbl = data->get_label_callback(widget, k);
            if (nk_console_list_view_item_matches(lbl, filter)) display_count++;
        }
        // If the currently selected item is no longer visible, move to first match.
        // When nothing matches, leave the selection untouched so it stays a valid
        // real index (nk_console_list_view_selected_label() reports NULL instead).
        if (display_count > 0) {
            const char* sel_label = data->get_label_callback(widget, data->selected);
            if (!nk_console_list_view_item_matches(sel_label, filter)) {
                data->selected = nk_console_list_view_nth_match(widget, data, 0, filter);
                nk_console_list_view_apply_scroll(data, 0);
            }
        }
    }

    // Display index of the currently selected item (used for scroll math).
    nk_uint sel_disp = nk_console_list_view_display_index(widget, data, data->selected, filter);

    // When calculating the height, consider the header height too.
    float header_height = 0.0f;
    if ((data->flags & (NK_WINDOW_TITLE | NK_WINDOW_CLOSABLE | NK_WINDOW_MINIMIZABLE)) &&
            !(data->flags & NK_WINDOW_HIDDEN) && widget->label != NULL) {
        header_height = top->ctx->style.font->height
            + 2.0f * top->ctx->style.window.header.padding.y
            + 2.0f * top->ctx->style.window.header.label_padding.y;
    }

    // Determine how many rows to show and the overall box height.
    nk_uint rows_visible = data->rows_visible;
    if (rows_visible == 0) {
        // Calling nk_layout_widget_bounds() here, before this widget lays out its
        // own row, reports the *previous* row (at_y only advances on the next
        // nk_layout_row_* call), so this widget's real top is peek.y + peek.h.
        struct nk_rect peek = nk_layout_widget_bounds(top->ctx);
        float list_top = peek.y + peek.h;
        struct nk_rect content = nk_window_get_content_region(top->ctx);
        // Leave the same padding gap at the bottom that the window reserves at the
        // top, and reserve room for the header bar so rows still fit.
        float remaining = (content.y + content.h) - list_top - top->ctx->style.window.padding.y - header_height;
        // Use a signed intermediate: casting a negative float to nk_uint is UB and
        // would slip past the < 1 clamp as a huge value.
        int fit = (int)(remaining / scroll_row_height);
        rows_visible = fit < 1 ? (nk_uint)1 : (nk_uint)fit;
    }
    float box_height = scroll_row_height * (float)rows_visible + header_height;

    if (widget->disabled) {
        nk_widget_disable_begin(top->ctx);
    }

    // Handle keyboard/gamepad navigation.
    if (is_active && !top_data->input_processed) {
        // Hold-to-accelerate timers - mirrors nk_console_check_up_down.
        nk_bool up_held = nk_console_button_down(top, NK_GAMEPAD_BUTTON_UP);
        nk_bool down_held = nk_console_button_down(top, NK_GAMEPAD_BUTTON_DOWN);
        nk_bool repeat_fire = nk_false;
        if (up_held || down_held) {
            if (top->ctx->delta_time_seconds > 0) {
                top_data->up_down_hold_timer += top->ctx->delta_time_seconds;
                top_data->up_down_repeat_timer += top->ctx->delta_time_seconds;
                float interval = NK_MAX(0.045f, 0.5f - top_data->up_down_hold_timer * 0.125f);
                if (top_data->up_down_repeat_timer >= interval) {
                    repeat_fire = nk_true;
                    top_data->up_down_repeat_timer -= interval;
                }
            }
        }
        else {
            top_data->up_down_hold_timer = 0;
            top_data->up_down_repeat_timer = 0;
        }

        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LB) || nk_input_is_key_pressed(&top->ctx->input, NK_KEY_SCROLL_UP)) {
            // Page up: jump selection up by rows_visible items (in display space).
            if (sel_disp > 0) {
                nk_uint new_disp = (nk_uint)NK_MAX(0, (int)sel_disp - (int)rows_visible);
                data->selected = nk_console_list_view_nth_match(widget, data, new_disp, filter);
                sel_disp = new_disp;
                nk_console_list_view_apply_scroll(data, sel_disp * (nk_uint)scroll_row_height);
            }
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RB) || nk_input_is_key_pressed(&top->ctx->input, NK_KEY_SCROLL_DOWN)) {
            // Page down: jump selection down by rows_visible items (in display space).
            if (display_count > 0 && sel_disp < display_count - 1) {
                nk_uint new_disp = NK_MIN(display_count - 1, sel_disp + rows_visible);
                data->selected = nk_console_list_view_nth_match(widget, data, new_disp, filter);
                sel_disp = new_disp;
                nk_console_list_view_scroll_into_view_down(data, sel_disp, display_count, scroll_row_height);
            }
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_UP) || (up_held && repeat_fire)) {
            if (sel_disp > 0) {
                // Find the previous matching item.
                nk_uint new_disp = sel_disp - 1;
                data->selected = nk_console_list_view_nth_match(widget, data, new_disp, filter);
                sel_disp = new_disp;
                if ((int)sel_disp < data->view.begin) {
                    nk_console_list_view_apply_scroll(data, sel_disp * (nk_uint)scroll_row_height);
                }
            }
            else {
                // At top: move focus to the previous sibling widget.
                nk_console_list_view_focus_sibling(widget, top_data, -1);
            }
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_DOWN) || (down_held && repeat_fire)) {
            if (display_count > 0 && sel_disp < display_count - 1) {
                nk_uint new_disp = sel_disp + 1;
                data->selected = nk_console_list_view_nth_match(widget, data, new_disp, filter);
                sel_disp = new_disp;
                // Scroll if selected is at or past the last fully-visible row.
                nk_console_list_view_scroll_into_view_down(data, sel_disp, display_count, scroll_row_height);
            }
            else {
                // At bottom: move focus to the next sibling widget.
                nk_console_list_view_focus_sibling(widget, top_data, +1);
            }
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT)) {
            // Move focus to the previous sibling widget.
            nk_console_list_view_focus_sibling(widget, top_data, -1);
            top_data->input_processed = nk_true;
        }
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT)) {
            // Move focus to the next sibling widget.
            nk_console_list_view_focus_sibling(widget, top_data, +1);
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

    // Display the visible rows. Lay out the row first, then read the real widget
    // bounds (matches every other console widget and yields an accurate return rect).
    nk_layout_row_dynamic(top->ctx, box_height, 1);
    struct nk_rect widget_bounds = nk_layout_widget_bounds(top->ctx);
    if (nk_list_view_begin(top->ctx, &data->view, widget->label, data->flags, (int)row_height, (int)display_count)) {
        // Cache the normal button style to restore it later on.
        struct nk_style_item saved_normal = top->ctx->style.button.normal;
        struct nk_color saved_text = top->ctx->style.button.text_normal;

        nk_layout_row_dynamic(top->ctx, row_height, 1);

        // Map the visible display window to real item indices with a single
        // forward scan (O(row_count) total) instead of an O(row_count) search per
        // visible row. First advance the cursor to the start of the window.
        nk_uint scan_k = 0; // real-index cursor, only used while a filter is active
        if (filter != NULL) {
            nk_uint matches_seen = 0;
            while (scan_k < data->row_count && matches_seen < (nk_uint)data->view.begin) {
                if (nk_console_list_view_item_matches(data->get_label_callback(widget, scan_k), filter)) {
                    matches_seen++;
                }
                scan_k++;
            }
        }

        for (int i = 0; i < data->view.count; ++i) {
            // Map display index to real item index.
            int real_i;
            if (filter != NULL) {
                // Advance to the next matching item.
                while (scan_k < data->row_count &&
                       !nk_console_list_view_item_matches(data->get_label_callback(widget, scan_k), filter)) {
                    scan_k++;
                }
                if (scan_k >= data->row_count) {
                    break; // Fewer matches than the view expected; stop safely.
                }
                real_i = (int)scan_k;
                scan_k++;
            }
            else {
                real_i = data->view.begin + i;
            }

            const char* label = data->get_label_callback(widget, (nk_uint)real_i);
            if (label == NULL) {
                label = "";
            }

            // Mouse Selection
            if (nk_input_is_mouse_moved(&top->ctx->input) && nk_widget_is_hovered(top->ctx) && !widget->disabled) {
                data->selected = (nk_uint)real_i;
                nk_console_set_active_widget(widget);
            }

            // Render each row as a selectable button, highlighting if selected.
            nk_bool is_selected = (nk_uint)real_i == data->selected;

            // Active items should get the active button style.
            if (is_selected && is_active) {
                top->ctx->style.button.normal = top->ctx->style.button.hover;
                top->ctx->style.button.text_normal = top->ctx->style.button.text_hover;
            }

            nk_bool mouse_clicked = nk_button_label(top->ctx, label);

            // Restore the original button style.
            if (is_selected && is_active) {
                top->ctx->style.button.normal = saved_normal;
                top->ctx->style.button.text_normal = saved_text;
            }

            if (mouse_clicked && top_data->input_processed == nk_false) {
                data->selected = (nk_uint)real_i;
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
    data->rows_visible = rows_visible < 0 ? (nk_uint)10 : (nk_uint)rows_visible;
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

NK_API void nk_console_list_view_set_searchable(nk_console* list_view, nk_bool searchable) {
    if (list_view == NULL || list_view->data == NULL || list_view->type != NK_CONSOLE_LIST_VIEW) {
        return;
    }
    nk_console_list_view_data* data = (nk_console_list_view_data*)list_view->data;
    if (data->searchable == searchable) {
        return;
    }
    data->searchable = searchable;

    if (searchable) {
        // Add a textedit sibling immediately before this list view (only once).
        if (data->search == NULL && list_view->parent != NULL) {
            nk_console* search = nk_console_textedit(list_view->parent, "Search", data->search_buffer, NK_CONSOLE_LIST_VIEW_SEARCH_BUFFER_SIZE);
            if (search != NULL) {
                int lv_idx = nk_console_get_widget_index(list_view);
                int last = (int)cvector_size(list_view->parent->children) - 1;
                if (lv_idx >= 0 && last > lv_idx) {
                    cvector_erase(list_view->parent->children, (size_t)last);
                    cvector_insert(list_view->parent->children, (size_t)lv_idx, search);
                }
                data->search = search;
            }
        }
    }
    else if (data->search != NULL) {
        // Remove and free the search field, then clear the active filter.
        nk_console* search = data->search;
        nk_console* parent = search->parent;
        data->search = NULL;
        if (parent != NULL) {
            // Drop any dangling references to the widget we're about to free.
            if (parent->activeWidget == search) {
                parent->activeWidget = NULL;
            }
            nk_console_top_data* top_data = (nk_console_top_data*)nk_console_get_top(list_view)->data;
            if (top_data != NULL && top_data->scroll_to_widget == search) {
                top_data->scroll_to_widget = NULL;
            }
            int idx = nk_console_get_widget_index(search);
            if (idx >= 0) {
                cvector_erase(parent->children, (size_t)idx);
            }
        }
        nk_console_free(search);
        data->search_buffer[0] = '\0';
    }
}

#endif /* NK_CONSOLE_LIST_VIEW_IMPLEMENTATION_ONCE */
#endif /* NK_CONSOLE_IMPLEMENTATION && !NK_CONSOLE_HEADER_ONLY */
