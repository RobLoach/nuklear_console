#ifndef NK_CONSOLE_ROW_H__
#define NK_CONSOLE_ROW_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_row_data {
  int activeChild;
} nk_console_row_data;

NK_API nk_console* nk_console_row_begin(nk_console* parent);
NK_API void nk_console_row_end(nk_console* console);
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

static inline nk_console* nk_console_row_active_child(nk_console* row) {
  nk_console_row_data* data = (nk_console_row_data*)row->data;
  NK_ASSERT(data->activeChild >= 0);
  NK_ASSERT((size_t)data->activeChild < cvector_size(row->children));
  return row->children[data->activeChild];
}

// Find the index of the next selectable child by either going right (direction
// = 1) or left (direction = -1). Returns the index of the currently active
// child if no other selectable child is found.
static int nk_console_row_next_selectable_child(nk_console* row,
                                                int direction) {
  nk_console_row_data* data = (nk_console_row_data*)row->data;
  int numChildren = (int)cvector_size(row->children);
  for (int i = data->activeChild + direction; i >= 0 && i < numChildren;
       i += direction) {
    if (row->children[i]->selectable) {
      return i;
    }
  }
  return data->activeChild;
}

static void nk_console_row_pick_nearest_selectable_child(nk_console* row) {
  nk_console_row_data* data = (nk_console_row_data*)row->data;
  NK_ASSERT(!nk_console_row_active_child(row)->selectable);
  int numChildren = (int)cvector_size(row->children);
  int index = data->activeChild;
  for (int i = 1; i < numChildren; ++i) {
    if (index + i < numChildren &&
        row->children[index + i]->selectable) {
      data->activeChild = index + i;
      break;
    }
    if (index - i >= 0 && row->children[index - i]->selectable) {
      data->activeChild = index - i;
      break;
    }
  }
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

NK_API void nk_console_row_end(nk_console* console) {
  nk_console_row_data* data = (nk_console_row_data*)console->data;
  size_t numChildren = cvector_size(console->children);
  if (!numChildren) {
    return;
  }

  // Take ownership of the number of columns
  for (size_t i = 0; i < numChildren; ++i) {
    nk_console* child = console->children[i];
    // This row is selectable iff there's at least one selectable child.
    console->selectable |= child->selectable;

    // Calculate the maximum amount of columns that are in the row
    console->columns += child->columns;
  }

  // Make sure we start on a selectable child by default.
  if (!nk_console_row_active_child(console)->selectable) {
    data->activeChild = nk_console_row_next_selectable_child(console, 1);
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
    nk_console_set_active_widget(console);

    // Find the child that the mouse is hovering over.
    float x = (input->mouse.pos.x - widget_bounds.x) / widget_bounds.w;
    data->activeChild = (int)(x * numChildren);

    // Ensure it's a valid index.
    if (data->activeChild >= numChildren) {
      data->activeChild = numChildren - 1;
    }
    else if (data->activeChild < 0) {
      data->activeChild = 0;
    }
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
    if (active != console &&
        active->type == NK_CONSOLE_ROW) {
    nk_console_row_data* activeData = (nk_console_row_data*)active->data;
      float x = (float)data->activeChild / cvector_size(console->children);
      activeData->activeChild = x * cvector_size(active->children);
      if (!nk_console_row_active_child(active)->selectable) {
        nk_console_row_pick_nearest_selectable_child(active);
      }
    }
  }

  if (!console->disabled && nk_console_is_active_widget(console) &&
      numChildren > 0) {
    console->activeWidget = nk_console_row_active_child(console);
  }

  for (int i = 0; i < numChildren; ++i) {
    nk_console* child = console->children[i];
    if (child->render) {
      child->render(child);
    }
  }
  console->activeWidget = NULL;

  if (console->disabled) {
    nk_widget_disable_end(console->context);
  }

  nk_layout_row_end(console->context);

  return widget_bounds;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_ROW_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
