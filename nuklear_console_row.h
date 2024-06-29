#ifndef NK_CONSOLE_ROW_H__
#define NK_CONSOLE_ROW_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_row_data {
  int activeChild;
} nk_console_row_data;

/**
 * Begin a new row in the console.
 *
 * @param parent The parent of which to create the new row.
 * @return The new row.
 */
NK_API nk_console* nk_console_row_begin(nk_console* parent);

/**
 * End the current row in the console.
 *
 * @param row The row to end.
 */
NK_API void nk_console_row_end(nk_console* row);
NK_API struct nk_rect nk_console_row_render(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_ROW_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_ROW_IMPLEMENTATION_ONCE
#define NK_CONSOLE_ROW_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

static nk_console* nk_console_row_active_child(nk_console* row) {
    if (row == NULL || row->data == NULL) {
        return NULL;
    }

    int amount = (int)cvector_size(row->children);
    if (amount == 0) {
        return NULL;
    }

    nk_console_row_data* data = (nk_console_row_data*)row->data;
    if (data->activeChild < 0) {
        data->activeChild = 0;
    }
    else if (data->activeChild >= amount) {
        data->activeChild = amount - 1;
    }

    return row->children[data->activeChild];
}

// Find the index of the next selectable child by either going right (direction
// = 1) or left (direction = -1). Returns the index of the currently active
// child if no other selectable child is found.
static int nk_console_row_next_selectable_child(nk_console* row, int direction) {
    nk_console_row_data* data = (nk_console_row_data*)row->data;
    int numChildren = (int)cvector_size(row->children);
    for (int i = data->activeChild + direction; i >= 0 && i < numChildren; i += direction) {
        if (row->children[i]->selectable) {
            return i;
        }
    }

    return data->activeChild;
}

/**
 * Pick the nearest selectable child to the currently active child.
 *
 * @param row The row to pick the nearest selectable child from.
 * @return True or false if a new selectable child was found.
 */
static nk_bool nk_console_row_pick_nearest_selectable_child(nk_console* row) {
    // Ensure there is actually a possible active child.
    if (row == NULL || row->data == NULL || nk_console_row_active_child(row) == NULL) {
        return nk_false;
    }

    nk_console_row_data* data = (nk_console_row_data*)row->data;
    int amount = (int)cvector_size(row->children);
    for (int i = 1; i < amount; ++i) {
        if (data->activeChild + i < amount && row->children[data->activeChild + i]->selectable) {
            data->activeChild += i;
            return nk_true;
        }
        if (data->activeChild - i >= 0 && row->children[data->activeChild - i]->selectable) {
            data->activeChild -= i;
            return nk_true;
        }
    }

    return nk_false;
}

NK_API nk_console* nk_console_row_begin(nk_console* parent) {
    // Create the row data.
    nk_handle unused = {0};
    nk_console_row_data* data = (nk_console_row_data*)NK_CONSOLE_MALLOC(unused, NULL, sizeof(nk_console_row_data));
    nk_zero(data, sizeof(nk_console_row_data));

    // Create the row.
    nk_console* row = nk_console_label(parent, NULL);
    row->data = data;
    row->alignment = NK_TEXT_ALIGN_CENTERED;
    row->type = NK_CONSOLE_ROW;
    row->render = nk_console_row_render;
    row->columns = 0;
    row->selectable = nk_false;
    return row;
}

NK_API void nk_console_row_end(nk_console* row) {
    // Make sure there is a row to end that has available children.
    nk_console* active_child = nk_console_row_active_child(row);
    if (active_child == NULL) {
        return;
    }

    // Set up the row data based on the available children.
    row->selectable = nk_false;
    int numChildren = (int)cvector_size(row->children);
    for (int i = 0; i < numChildren; ++i) {
        nk_console* child = row->children[i];

        // This row is selectable if there's at least one selectable child.
        row->selectable |= child->selectable;

        // Calculate the maximum amount of columns that are in the row
        row->columns += child->columns;
    }

    // Make sure we start on a selectable child by default.
    if (!active_child->selectable) {
        nk_console_row_pick_nearest_selectable_child(row);
    }
}

static void nk_console_row_check_left_right(nk_console* row, nk_console* top) {
    nk_console_row_data* data = (nk_console_row_data*)row->data;
    if (top->input_processed) {
        return;
    }

    // Left
    if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT)) {
        data->activeChild = nk_console_row_next_selectable_child(row, -1);
        top->input_processed = nk_true;
    }
    // Right
    else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT)) {
        data->activeChild = nk_console_row_next_selectable_child(row, 1);
        top->input_processed = nk_true;
    }
}

NK_API struct nk_rect nk_console_row_render(nk_console* console) {
    if (console == NULL || console->data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_row_data* data = (nk_console_row_data*)console->data;
    nk_console* top = nk_console_get_top(console);

    // Rows use the advanced layout system to render their children.
    nk_layout_row_begin(console->context, NK_DYNAMIC, 0, console->columns);

    struct nk_rect widget_bounds = nk_layout_widget_bounds(console->context);

    if (console->disabled) {
        nk_widget_disable_begin(console->context);
    }

    // Consume mouse movement before children have a chance to.
    int numChildren = (int)cvector_size(console->children);
    struct nk_input* input = &console->context->input;
    if (console->selectable && top->input_processed == nk_false &&
        widget_bounds.w > 0 && nk_input_is_mouse_moved(input) &&
        nk_input_is_mouse_hovering_rect(input, widget_bounds)) {
        // First, make sure that the active widget is the row.
        nk_console_set_active_widget(console);

        // Find the child that the mouse is hovering over.
        if (console->columns > 0) {
            float x_percent = (input->mouse.pos.x - widget_bounds.x) / widget_bounds.w;
            float column_width_percent = 0.0f;
            for (int i = 0; i < numChildren; i++) {
                int widget_columns = console->children[i]->columns;
                if (widget_columns <= 0) {
                    widget_columns = 1;
                }
                column_width_percent += (float)widget_columns / (float)console->columns;
                if (x_percent < column_width_percent) {
                    data->activeChild = i;
                    break;
                }
            }
        }

        // Ensure the new active child is actually selectable.
        if (!nk_console_row_active_child(console)->selectable) {
            nk_console_row_pick_nearest_selectable_child(console);
        }
    }

    // Consume directional input before children have a chance to.
    if (nk_console_is_active_widget(console)) {
        nk_console_row_check_left_right(console, top);
        nk_console_check_up_down(console, widget_bounds);
        nk_console* active = nk_console_get_active_widget(console);
        NK_ASSERT(active);
        // Attempt to accuratle move vertically if the new widget is also a row.
        // TODO: Pick the nearest selectable child based on the X percent of the columns, rather than the number of children.
        if (active != console && active->type == NK_CONSOLE_ROW) {
            nk_console_row_data* activeData = (nk_console_row_data*)active->data;
            float x = (float)data->activeChild / (float)numChildren;
            activeData->activeChild = x * (float)cvector_size(active->children);
            if (!nk_console_row_active_child(active)->selectable) {
                nk_console_row_pick_nearest_selectable_child(active);
            }
        }
    }

    // Set the active widget to the active child of the row.
    if (!console->disabled && nk_console_is_active_widget(console) && numChildren > 0) {
        console->activeWidget = nk_console_row_active_child(console);
    }

    // Render all the children
    for (int i = 0; i < numChildren; ++i) {
        nk_console* child = console->children[i];
        if (child->render != NULL) {
            child->render(child);
        }
    }

    console->activeWidget = NULL;

    if (console->disabled) {
        nk_widget_disable_end(console->context);
    }

    // Finished rendering the row, so complete the row layout.
    nk_layout_row_end(console->context);

    return widget_bounds;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_ROW_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
