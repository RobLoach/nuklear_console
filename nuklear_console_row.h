#ifndef NK_CONSOLE_ROW_H__
#define NK_CONSOLE_ROW_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_row_child {
  nk_console* console;
  int columns;
} nk_console_row_child;

typedef struct nk_console_row_data {
  int activeChild;
  nk_console_row_child* children;
} nk_console_row_data;

typedef struct nk_console_row {
  nk_console console;
  nk_console_row_data data;
} nk_console_row;

NK_API nk_console* nk_console_row_begin(nk_console* parent);
NK_API void nk_console_row_end(nk_console* console);
NK_API struct nk_rect nk_console_row_render(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_ROW_H__

#ifdef NK_CONSOLE_IMPLEMENTATION
#ifndef NK_CONSOLE_ROW_IMPLEMENTATION_ONCE
#define NK_CONSOLE_ROW_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

static inline nk_console* nk_console_row_active_child(nk_console_row* row) {
  NK_ASSERT(row->data.activeChild >= 0);
  NK_ASSERT((size_t)row->data.activeChild < cvector_size(row->data.children));
  return row->data.children[row->data.activeChild].console;
}

// Find the index of the next selectable child by either going right (direction
// = 1) or left (direction = -1). Returns the index of the currently active
// child if no other selectable child is found.
static int nk_console_row_next_selectable_child(nk_console_row* row,
                                                int direction) {
  int numChildren = (int)cvector_size(row->data.children);
  for (int i = row->data.activeChild + direction; i >= 0 && i < numChildren;
       i += direction) {
    if (row->data.children[i].console->selectable) {
      return i;
    }
  }
  return row->data.activeChild;
}

static void nk_console_row_pick_nearest_selectable_child(nk_console_row* row) {
  NK_ASSERT(!nk_console_row_active_child(row)->selectable);
  int numChildren = (int)cvector_size(row->data.children);
  int index = row->data.activeChild;
  for (int i = 1; i < numChildren; ++i) {
    if (index + i < numChildren &&
        row->data.children[index + i].console->selectable) {
      row->data.activeChild = index + i;
      break;
    }
    if (index - i >= 0 && row->data.children[index - i].console->selectable) {
      row->data.activeChild = index - i;
      break;
    }
  }
}

static void nk_console_destroy_row(nk_console* console) {
  nk_console_row* row = (nk_console_row*)console;
  size_t numChildren = cvector_size(row->data.children);
  for (size_t i = 0; i < numChildren; ++i) {
    nk_console_free(row->data.children[i].console);
  }
  cvector_free(row->data.children);
}

// Ensure it's safe to return an nk_console_row* as an nk_console*.
NK_STATIC_ASSERT(offsetof(nk_console_row, console) == 0);

NK_API nk_console* nk_console_row_begin(nk_console* parent) {
  nk_handle handle;
  nk_console_row* row = (nk_console_row*)nk_console_malloc(handle, NULL, sizeof(*row));
  nk_zero(row, sizeof(*row));
  row->console.context = parent->context;
  row->console.alignment = NK_TEXT_ALIGN_CENTERED;
  row->console.type = NK_CONSOLE_ROW;
  row->console.parent = parent;
  row->console.render = nk_console_row_render;
  row->console.destroy = &nk_console_destroy_row;
  cvector_push_back(parent->children, (nk_console*)row);
  NK_ASSERT((void*)&row->console == (void*)row);
  return &row->console;
}

NK_API void nk_console_row_end(nk_console* console) {
  nk_console_row* row = (nk_console_row*)console;
  size_t numChildren = cvector_size(console->children);
  if (!numChildren) {
    return;
  }
  // Move children into row->data. The row widget will be responsible for laying
  // out the correct number of columns.
  for (size_t i = 0; i < numChildren; ++i) {
    nk_console* child = console->children[i];
    // This row is selectable iff there's at least one selectable child.
    console->selectable |= child->selectable;
    console->columns += child->columns;
    nk_console_row_child info = {child, child->columns};
    child->columns = 0;  // The child should not set a row layout.
    cvector_push_back(row->data.children, info);
  }
  cvector_free(console->children);
  console->children = NULL;
  // Make sure we start on a selectable child by default.
  if (!nk_console_row_active_child(row)->selectable) {
    row->data.activeChild = nk_console_row_next_selectable_child(row, 1);
  }
}

static void nk_console_row_check_left_right(nk_console_row* row,
                                            nk_console* top) {
  NK_UNUSED(row);
  if (top->input_processed) {
    return;
  }
  // Left
  if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LEFT)) {
    row->data.activeChild = nk_console_row_next_selectable_child(row, -1);
    top->input_processed = nk_true;
  }
  // Right
  else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RIGHT)) {
    row->data.activeChild = nk_console_row_next_selectable_child(row, 1);
    top->input_processed = nk_true;
  }
}

NK_API struct nk_rect nk_console_row_render(nk_console* console) {
  nk_console_row* row = (nk_console_row*)console;
  nk_console* top = nk_console_get_top(console);

  nk_layout_row_dynamic(console->context, 0, console->columns);

  struct nk_rect widget_bounds = nk_layout_widget_bounds(console->context);

  if (console->disabled) {
    nk_widget_disable_begin(console->context);
  }

  // Consume mouse movement before children have a chance to.
  int numChildren = (int)cvector_size(row->data.children);
  struct nk_input* input = &console->context->input;
  if (console->selectable && top->input_processed == nk_false &&
      widget_bounds.w > 0 && nk_input_is_mouse_moved(input) &&
      nk_input_is_mouse_hovering_rect(input, widget_bounds)) {
    nk_console_set_active_widget(console);
    // Calculate the active child index based on where the mouse is within
    // widget_bounds. This assumes each column has equal width.
    float columnWidth = (float)widget_bounds.w / console->columns;
    float mouseX = input->mouse.pos.x - widget_bounds.x;
    row->data.activeChild = 0;
    for (float x = 0; x < widget_bounds.w;) {
      float right =
          x + row->data.children[row->data.activeChild].columns * columnWidth;
      if (right > mouseX) {
        break;
      }
      x = right;
      row->data.activeChild += 1;
    }
    if (!nk_console_row_active_child(row)->selectable) {
      nk_console_row_pick_nearest_selectable_child(row);
    }
    top->input_processed = nk_true;
  }

  // Consume directional input before children have a chance to.
  if (nk_console_is_active_widget(console)) {
    nk_console_row_check_left_right(row, top);
    nk_console_check_up_down(console, widget_bounds);
    nk_console* active = nk_console_get_active_widget(console);
    NK_ASSERT(active);
    // Attempt to accuratle move vertically if the new widget is also a row.
    if (active != console &&
        active->type == NK_CONSOLE_ROW) {
      nk_console_row* r2 = (nk_console_row*)active;
      float x = (float)row->data.activeChild / cvector_size(row->data.children);
      r2->data.activeChild = x * cvector_size(r2->data.children);
      if (!nk_console_row_active_child(r2)->selectable) {
        nk_console_row_pick_nearest_selectable_child(r2);
      }
    }
  }

  if (!console->disabled && nk_console_is_active_widget(console) &&
      numChildren > 0) {
    console->activeWidget = nk_console_row_active_child(row);
  }

  for (int i = 0; i < numChildren; ++i) {
    nk_console* child = row->data.children[i].console;
    if (child->render) {
      child->render(child);
    }
  }
  console->activeWidget = NULL;

  if (console->disabled) {
    nk_widget_disable_end(console->context);
  }

  return widget_bounds;
}

#if defined(__cplusplus)
}
#endif

#endif
#endif
