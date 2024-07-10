#ifndef NK_CONSOLE_H__
#define NK_CONSOLE_H__

#ifdef __cplusplus
extern "C" {
#endif

struct nk_console;

/**
 * Event handler for a console widget.
 *
 * @param widget The widget that was acted upon.
 */
typedef void (*nk_console_event)(struct nk_console* widget);

/**
 * An event handler for rendering the given widget.
 *
 * @param widget The widget that is being rendered.
 *
 * @return The bounds of the widget.
 */
typedef struct nk_rect (*nk_console_render_event)(struct nk_console* widget);

typedef enum {
    NK_CONSOLE_UNKNOWN,
    NK_CONSOLE_PARENT,
    NK_CONSOLE_LABEL,
    NK_CONSOLE_BUTTON,
    NK_CONSOLE_CHECKBOX,
    NK_CONSOLE_PROGRESS,
    NK_CONSOLE_COMBOBOX,
    NK_CONSOLE_PROPERTY_INT,
    NK_CONSOLE_PROPERTY_FLOAT,
    NK_CONSOLE_SLIDER_INT,
    NK_CONSOLE_SLIDER_FLOAT,
    NK_CONSOLE_ROW,
    NK_CONSOLE_TEXTEDIT,
    NK_CONSOLE_TEXTEDIT_TEXT
} nk_console_widget_type;

typedef struct nk_console {
    nk_console_widget_type type;
    void* user_data;
    const char* label;
    int label_length;
    int alignment;

    nk_bool selectable; /** Whether or not the widget can be selected. */
    nk_bool disabled; /** Whether or not the widget is currently disabled. */
    int columns; /** When set, will determine how many dynamic columns to set to for the active row. */
    int height; /** When set, will determine the height of the row. */
    const char* tooltip; /** Tooltip */
    void* data; /** Widget-specific data */

    struct nk_console* parent;
    struct nk_context* ctx;
    struct nk_console** children;
    struct nk_console* activeParent;
    struct nk_console* activeWidget;
    nk_bool input_processed;

    // Events
    nk_console_event onchange; /** Invoked when there is a change in the value for the widget. */
    nk_console_render_event render; /** Render the widget. */
    nk_console_event destroy; /** Destroy the widget. */

    void* gamepads;
} nk_console;

// Console
NK_API nk_console* nk_console_init(struct nk_context* context);
NK_API void nk_console_free(nk_console* console);
NK_API void nk_console_render(nk_console* console);

// Utilities
NK_API nk_console* nk_console_get_top(nk_console* widget);
NK_API int nk_console_get_widget_index(nk_console* widget);
NK_API void nk_console_check_tooltip(nk_console* console);
NK_API void nk_console_check_up_down(nk_console* widget, struct nk_rect bounds);
NK_API nk_bool nk_console_is_active_widget(nk_console* widget);
NK_API void nk_console_set_active_parent(nk_console* new_parent);
NK_API void nk_console_set_active_widget(nk_console* widget);
NK_API nk_console* nk_console_get_active_widget(nk_console* widget);
NK_API void* nk_console_malloc(nk_handle unused, void *old, nk_size size);
NK_API void nk_console_mfree(nk_handle unused, void *ptr);
NK_API nk_bool nk_console_button_pushed(nk_console* console, int button);
NK_API void nk_console_set_gamepads(nk_console* console, void* gamepads);
NK_API void nk_console_set_tooltip(nk_console* widget, const char* tooltip);
NK_API void nk_console_set_onchange(nk_console* widget, nk_console_event onchange);
NK_API void nk_console_set_label(nk_console* widget, const char* label, int label_length);
NK_API const char* nk_console_get_label(nk_console* widget);
NK_API void nk_console_free_children(nk_console* console);
NK_API void nk_console_layout_widget(nk_console* widget);
NK_API struct nk_rect nk_console_parent_render(nk_console* parent);
NK_API void nk_console_add_child(nk_console* parent, nk_console* child);
NK_API void nk_console_set_height(nk_console* widget, int height);
NK_API int nk_console_height(nk_console* widget);

#define NK_CONSOLE_HEADER_ONLY
#include "nuklear_console_label.h"
#include "nuklear_console_button.h"
#include "nuklear_console_checkbox.h"
#include "nuklear_console_progress.h"
#include "nuklear_console_combobox.h"
#include "nuklear_console_property.h"
#include "nuklear_console_row.h"
#include "nuklear_console_textedit.h"
#include "nuklear_console_textedit_text.h"
#undef NK_CONSOLE_HEADER_ONLY

#ifdef __cplusplus
}
#endif

#endif  // NK_CONSOLE_H__

#ifdef NK_CONSOLE_IMPLEMENTATION
#ifndef NK_CONSOLE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_IMPLEMENTATION_ONCE

// NK_CONSOLE_MALLOC
#ifndef NK_CONSOLE_MALLOC
#ifdef NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_CONSOLE_MALLOC nk_malloc
#else
#error "Requires NK_CONSOLE_MALLOC, or NK_INCLUDE_DEFAULT_ALLOCATOR"
#endif
#endif

// NK_CONSOLE_FREE
#ifndef NK_CONSOLE_FREE
#ifdef NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_CONSOLE_FREE nk_mfree
#else
#error "Requires NK_CONSOLE_FREE, or NK_INCLUDE_DEFAULT_ALLOCATOR"
#endif
#endif

#ifndef cvector_clib_free
#define cvector_clib_free(ptr) nk_console_mfree((nk_handle) {.id = 0}, ptr)
#endif
#ifndef cvector_clib_malloc
#define cvector_clib_malloc(size) nk_console_malloc((nk_handle) {.ptr = NULL}, NULL, size)
#endif
#ifndef cvector_clib_calloc
#define cvector_clib_calloc(count, size) NK_ASSERT(0 && "cvector_clib_calloc is not supported")
#endif
#ifndef cvector_clib_realloc
static void* nk_console_realloc(void* ptr, size_t size) {
	if (size == 0) {
        cvector_clib_free(ptr);
        return NULL;
    }
    if (ptr == NULL) {
        return cvector_clib_malloc(size);
    }
    void* output = cvector_clib_malloc(size);
    if (output == NULL) {
        return NULL;
    }
    NK_MEMCPY(output, ptr, size); // TODO: This memory copy is unsafe, and slow.
    cvector_clib_free(ptr);
	return output;
}
#define cvector_clib_realloc(ptr, size) nk_console_realloc(ptr, size)
#endif
#ifndef cvector_clib_assert
#define cvector_clib_assert(expression) NK_ASSERT(expression)
#endif
#ifndef cvector_clib_memcpy
#define cvector_clib_memcpy(dest, src, count) NK_MEMCPY(dest, src, count)
#endif
#ifndef cvector_clib_memmove
#define cvector_clib_memmove(dest, src, count) NK_ASSERT(0 && "cvector_clib_memmove is not supported")
#endif
#ifndef CVECTOR_H
#define CVECTOR_H "vendor/c-vector/cvector.h"
#endif
#include CVECTOR_H

#ifndef NK_CONSOLE_GAMEPAD_IS_BUTTON_PRESSED
#ifdef NUKLEAR_GAMEPAD_H__ // nuklear_gamepad.h
/**
 * Check if the given button is pressed on the gamepad.
 *
 * @param gamepads The gamepad system provided at `console->gamepads`.
 * @param index The index of the gamepad to check. -1 is provided to check all gamepads.
 * @param button Which button we are to check.
 *
 * @see nk_gamepad_is_button_pressed()
 * @see nk_gamepad_button
 * @see https://github.com/RobLoach/nuklear_gamepad
 */
#define NK_CONSOLE_GAMEPAD_IS_BUTTON_PRESSED(gamepads, index, button) nk_gamepad_is_button_pressed((struct nk_gamepads*)gamepads, index, button)
#endif  // NUKLEAR_GAMEPAD_H__
#endif  // NK_CONSOLE_GAMEPAD_IS_BUTTON_PRESSED

#ifdef __cplusplus
extern "C" {
#endif

NK_API nk_bool nk_input_is_mouse_moved(const struct nk_input* input);

#include "nuklear_console_label.h"
#include "nuklear_console_button.h"
#include "nuklear_console_checkbox.h"
#include "nuklear_console_progress.h"
#include "nuklear_console_combobox.h"
#include "nuklear_console_property.h"
#include "nuklear_console_row.h"
#include "nuklear_console_textedit_text.h"
#include "nuklear_console_textedit.h"

NK_API const char* nk_console_get_label(nk_console* widget) {
    if (widget == NULL) {
        return NULL;
    }

    return widget->label;
}

NK_API void nk_console_set_label(nk_console* widget, const char* label, int label_length) {
    if (widget == NULL) {
        return;
    }

    widget->label = label;
    widget->label_length = label_length;
}

NK_API void nk_console_set_tooltip(nk_console* widget, const char* tooltip) {
    if (widget == NULL) {
        return;
    }

    widget->tooltip = tooltip;
}

NK_API void nk_console_set_onchange(nk_console* widget, nk_console_event onchange) {
    if (widget == NULL) {
        return;
    }

    widget->onchange = onchange;
}

NK_API void nk_console_set_active_widget(nk_console* widget) {
    if (widget == NULL) {
        return;
    }

    nk_console* parent = widget->parent == NULL ? widget : widget->parent;
    parent->activeWidget = widget;
}

NK_API nk_console* nk_console_get_active_widget(nk_console* widget) {
    if (widget == NULL) {
        return NULL;
    }
    nk_console* parent = widget->parent == NULL ? widget : widget->parent;
    return parent->activeWidget;
}

NK_API nk_bool nk_console_is_active_widget(nk_console* widget) {
    if (widget == NULL) {
        return nk_false;
    }

    nk_console* parent = widget->parent == NULL ? widget : widget->parent;
    return parent->activeWidget == widget;
}

NK_API void nk_console_set_active_parent(nk_console* new_parent) {
    if (new_parent == NULL) {
        return;
    }

    nk_console* top = nk_console_get_top(new_parent);
    if (top != NULL) {
        top->activeParent = new_parent;
    }
}

/**
 * Get the top parent from a given widget.
 */
NK_API nk_console* nk_console_get_top(nk_console* widget) {
    if (widget == NULL) {
        return NULL;
    }

    while (widget->parent != NULL) {
        widget = widget->parent;
    }
    return widget;
}

NK_API void nk_console_set_height(nk_console* widget, int height) {
    if (widget == NULL) {
        return;
    }
    widget->height = height < 0 ? 0 : widget->height;
}

NK_API int nk_console_height(nk_console* widget) {
    if (widget == NULL) {
        return 0;
    }
    return widget->height;
}

/**
 * Get the given widget's index from it's parent's children.
 *
 * @return The index of the widget, or -1 on error
 */
NK_API int nk_console_get_widget_index(nk_console* widget) {
    if (widget == NULL) {
        return -1;
    }

    nk_console* parent = widget->parent;
    if (parent == NULL) {
        return -1;
    }

    size_t i;
    for (i = 0; i < cvector_size(parent->children); i++) {
        if (parent->children[i] == widget) {
            return i;
        }
    }

    return -1;
}

/**
 * Allow the user to move up and down between widgets.
 */
NK_API void nk_console_check_up_down(nk_console* widget, struct nk_rect bounds) {
    nk_console* top = nk_console_get_top(widget);

    // Scroll to the active widget if needed.
    struct nk_rect content_region = nk_window_get_content_region(widget->ctx);
    nk_uint offsetx, offsety;
    nk_window_get_scroll(widget->ctx, &offsetx, &offsety);
    if (bounds.y + bounds.h > content_region.y + content_region.h + offsety) {
        nk_uint dest = bounds.y + bounds.h - content_region.y - content_region.h;
        nk_window_set_scroll(widget->ctx, offsetx, dest);
    }
    else if (bounds.y < content_region.y + offsety) {
        nk_uint dest = bounds.y - content_region.y;
        nk_window_set_scroll(widget->ctx, offsetx, dest);
    }

    // Only process an active input once.
    if (top->input_processed == nk_false) {
        // Page Up
        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LB)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            int count = 0;
            while (--widgetIndex >= 0) {
                nk_console* target = widget->parent->children[widgetIndex];
                if (target != NULL && target->selectable && !target->disabled) {
                    nk_console_set_active_widget(target);
                    if (++count > 4) {
                        break;
                    }
                }
            }
            top->input_processed = nk_true;
        }
        // Page Down
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RB)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            int count = 0;
            while (++widgetIndex < (int)cvector_size(widget->parent->children)) {
                nk_console* target = widget->parent->children[widgetIndex];
                if (target != NULL && target->selectable && !target->disabled) {
                    nk_console_set_active_widget(target);
                    if (++count > 4) {
                        break;
                    }
                }
            }
            top->input_processed = nk_true;
        }
        // Up
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_UP)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            while (--widgetIndex >= 0) {
                nk_console* target = widget->parent->children[widgetIndex];
                if (target != NULL && target->selectable && !target->disabled) {
                    nk_console_set_active_widget(target);
                    break;
                }
            }
            top->input_processed = nk_true;
        }
        // Down
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_DOWN)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            while (++widgetIndex < (int)cvector_size(widget->parent->children)) {
                nk_console* target = widget->parent->children[widgetIndex];
                if (target != NULL && target->selectable && !target->disabled) {
                    nk_console_set_active_widget(target);
                    break;
                }
            }
            top->input_processed = nk_true;
        }
        // Back
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_B)) {
            if (top->activeParent == NULL) {
                return;
            }

            if (widget->parent != NULL) {
                if (widget->parent == top) {
                    top->activeParent = top;
                }
                else if (widget->parent->parent != NULL) {
                    top->activeParent = widget->parent->parent;
                }
            }

            top->input_processed = nk_true;
        }
    }
}

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
            if (parent->children[i]->selectable && !parent->children[i]->disabled) {
                return parent->children[i];
            }
        }
    }

    return NULL;
}

/**
 * A function to check whether or not the mouse moved.
 */
NK_API nk_bool nk_input_is_mouse_moved(const struct nk_input* input) {
    if (input == NULL) {
        return nk_false;
    }

    return input->mouse.delta.x != 0 || input->mouse.delta.y != 0;
}

/**
 * Display a tooltip with the given text.
 *
 * @see nk_tooltip()
 * @todo Support multiline tooltips with nk_text_calculate_text_bounds()
 */
static void nk_console_tooltip_display(struct nk_context *ctx, const char *text) {
    const struct nk_style *style;
    struct nk_vec2 padding;

    style = &ctx->style;
    padding = style->window.padding;

    float text_height = (style->font->height + padding.y);
    int x = ctx->input.mouse.pos.x;
    int y = ctx->input.mouse.pos.y;

    // Display the tooltip at the bottom of the window, manipulating the mouse position
    struct nk_rect windowbounds = nk_window_get_bounds(ctx);
    ctx->input.mouse.pos.x = windowbounds.x;
    ctx->input.mouse.pos.y = windowbounds.y + windowbounds.h - text_height - padding.y * 2;

    if (nk_tooltip_begin(ctx, (float)windowbounds.w)) {
        nk_layout_row_dynamic(ctx, text_height, 1);
        nk_text_wrap(ctx, text, nk_strlen(text));
        nk_tooltip_end(ctx);
    }

    // Restore the mouse x/y positions.
    ctx->input.mouse.pos.x = x;
    ctx->input.mouse.pos.y = y;
}

NK_API void nk_console_check_tooltip(nk_console* console) {
    if (console == NULL) {
        return;
    }

    if (console->tooltip != NULL) {
        nk_console_tooltip_display(console->ctx, console->tooltip);
    }
}

/**
 * Render the given console widget.
 *
 * @param console The console widget to display.
 */
NK_API void nk_console_render(nk_console* console) {
    if (console == NULL) {
        return;
    }

    // First run
    if (console->parent == NULL) {
        // Reset the input state.
        console->input_processed = nk_false;

        // Make sure there is an active widget.
        if (console->activeWidget == NULL) {
            nk_console_set_active_widget(nk_console_find_first_selectable(console->activeParent != NULL ? console->activeParent : console));
        }

        // Render the active parent.
        if (console->activeParent != NULL && console->activeParent->children != NULL) {
            // Make sure there's an active widget selected.
            if (console->activeParent->activeWidget == NULL) {
                nk_console_set_active_widget(nk_console_find_first_selectable(console->activeParent));
            }
            else {
                nk_bool widgetFound = nk_false;
                for (size_t i = 0; i < cvector_size(console->activeParent->children); ++i) {
                    if (console->activeParent->children[i] == console->activeParent->activeWidget) {
                        widgetFound = nk_true;
                        break;
                    }
                }

                if (widgetFound == nk_false) {
                    nk_console_set_active_widget(nk_console_find_first_selectable(console->activeParent));
                }
            }

            // Render all the children
            for (size_t i = 0; i < cvector_size(console->activeParent->children); ++i) {
                nk_console_render(console->activeParent->children[i]);
            }
            return;
        }
    }

    // Render the widget and get its bounds.
    struct nk_rect widget_bounds = console->render != NULL ? console->render(console) : nk_rect(0, 0, 0, 0);

    // Allow the mouse to switch focus to the widget.
    if (widget_bounds.w > 0 && widget_bounds.h > 0 && nk_input_is_mouse_moved(&console->ctx->input)) {
        // Make sure we consider the active scroll position of the window.
        nk_uint window_scroll_x, window_scroll_y;
        nk_window_get_scroll(console->ctx, &window_scroll_x, &window_scroll_y);
        widget_bounds.x -= (float)window_scroll_x;
        widget_bounds.y -= (float)window_scroll_y;

        nk_console* top = nk_console_get_top(console);
        if (top->input_processed == nk_false && nk_input_is_mouse_hovering_rect(&console->ctx->input, widget_bounds)) {
            // Select the widget, if possible.
            if (!console->disabled && console->selectable) {
                nk_console_set_active_widget(console);
                top->input_processed = nk_true;
            }
        }
    }
}

NK_API void* nk_console_malloc(nk_handle unused, void *old, nk_size size) {
    NK_UNUSED(unused);
    NK_UNUSED(old);
    return NK_CONSOLE_MALLOC(unused, old, size);
}

NK_API void nk_console_mfree(nk_handle unused, void *ptr) {
    NK_UNUSED(unused);
    NK_CONSOLE_FREE(unused, ptr);
}

/**
 * Render all the children of the given parent.
 */
NK_API struct nk_rect nk_console_parent_render(nk_console* parent) {
    if (parent == NULL || parent->children == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    // // Render the children
    int children_size = (int)cvector_size(parent->children);
    for (int i = 0; i < children_size; i++) {
        nk_console_render(parent->children[i]);
    }

    return nk_rect(0, 0, 0, 0);
}

/**
 * Initialize a new nk_console.
 *
 * @param context The associated Nuklear context.
 */
NK_API nk_console* nk_console_init(struct nk_context* context) {
    nk_handle handle;
    nk_console* console = nk_console_malloc(handle, NULL, sizeof(nk_console));
    nk_zero(console, sizeof(nk_console));
    console->type = NK_CONSOLE_PARENT;
    console->ctx = context;
    console->alignment = NK_TEXT_ALIGN_CENTERED;
    console->render = nk_console_parent_render;
    return console;
}

/**
 * Free the given nk_console's data.
 */
NK_API void nk_console_free(nk_console* console) {
    if (console == NULL) {
        return;
    }
    nk_handle handle = {0};

    if (console->destroy) {
        console->destroy(console);
    }

    // Clear any component-specific data.
    if (console->data != NULL) {
        nk_console_mfree(handle, console->data);
        console->data = NULL;
    }

    // Clear all the children
    nk_console_free_children(console);

    nk_console_mfree(handle, console);
}

NK_API void nk_console_free_children(nk_console* console) {
    if (console == NULL) {
        return;
    }

    // Since there won't be any children, make sure to unselect any active child.
    console->activeWidget = NULL;

    // Clear all the children
    if (console->children != NULL) {
		for (size_t i = 0; i < cvector_size(console->children); ++i) {
            nk_console_free(console->children[i]);
		}
    }

    cvector_free(console->children);
    console->children = NULL;
}

/**
 * Set up the layout of a widget, based on the amount of columns specified.
 *
 * @see nk_layout_row_dynamic()
 * @see nk_layout_row_push()
 * @see nk_console::columns
 */
NK_API void nk_console_layout_widget(nk_console* widget) {
    // If the widget has 0 columns, don't do anything.
    if (widget == NULL || widget->columns <= 0) {
        return;
    }

    // If we're rendering a row, use the parent's columns to determine the width of the widget.
    if (widget->parent != NULL && widget->parent->type == NK_CONSOLE_ROW) {
        // Calculate how wide the widget's column should be.
        if (widget->parent->columns > 0) { // Avoid division by 0
            nk_layout_row_push(widget->ctx, (float)widget->columns / (float)widget->parent->columns);
        }

        return;
    }

    // Since we're not within a row, the widget owns the whole row.
    nk_layout_row_dynamic(widget->ctx, widget->height, widget->columns);
}

NK_API void nk_console_set_gamepads(nk_console* console, void* gamepads) {
    if (console == NULL) {
        return;
    }

    console->gamepads = gamepads;
}

NK_API nk_bool nk_console_button_pushed(nk_console* console, int button) {
    if (console == NULL) {
        return nk_false;
    }

    // Get the top console.
    if (console->parent != NULL) {
        console = nk_console_get_top(console);
    }

    // Check gamepads.
    #ifdef NK_CONSOLE_GAMEPAD_IS_BUTTON_PRESSED
        if (NK_CONSOLE_GAMEPAD_IS_BUTTON_PRESSED(console->gamepads, -1, button)) {
            return nk_true;
        }
    #endif

    // Keyboard/Mouse
    switch (button) {
        case NK_GAMEPAD_BUTTON_UP: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_UP);
        case NK_GAMEPAD_BUTTON_DOWN: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_DOWN);
        case NK_GAMEPAD_BUTTON_LEFT: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_LEFT);
        case NK_GAMEPAD_BUTTON_RIGHT: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_RIGHT);
        case NK_GAMEPAD_BUTTON_A: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_ENTER);
        case NK_GAMEPAD_BUTTON_B: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_BACKSPACE) || (nk_input_is_mouse_pressed(&console->ctx->input, NK_BUTTON_RIGHT) && nk_window_is_hovered(console->ctx));
        // case NK_GAMEPAD_BUTTON_X: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_A);
        // case NK_GAMEPAD_BUTTON_Y: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_S);
        case NK_GAMEPAD_BUTTON_LB: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_DOWN) && nk_input_is_key_down(&console->ctx->input, NK_KEY_CTRL);
        case NK_GAMEPAD_BUTTON_RB: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_UP) && nk_input_is_key_down(&console->ctx->input, NK_KEY_CTRL);
        case NK_GAMEPAD_BUTTON_BACK: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_SHIFT);
        //case NK_GAMEPAD_BUTTON_START: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_UP);
    }

    return nk_false;
}

NK_API void nk_console_add_child(nk_console* parent, nk_console* child) {
    if (parent == NULL || child == NULL) {
        return;
    }

    child->parent = parent;
    cvector_push_back(parent->children, child);

    // If we added a child to a row, make sure to let the row know a new widget was added.
    if (parent->type == NK_CONSOLE_ROW) {
        struct nk_console_row_data* data = (struct nk_console_row_data*)parent->data;
        if (data != NULL && data->widgets_added == nk_true) {
            nk_console_row_end(parent);
        }
    }
}

#ifdef __cplusplus
}
#endif

#endif
#endif
