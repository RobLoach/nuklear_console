#ifndef NK_CONSOLE_H__
#define NK_CONSOLE_H__

struct nk_console;

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
    NK_CONSOLE_SLIDER_FLOAT
} nk_console_widget_type;

typedef struct nk_console_combobox_data {
    const char* label;
    const char* items_separated_by_separator;
    int separator;
    int* selected;
    int count;
} nk_console_combobox_data;

/**
 * Property type, also used for slider.
 */
typedef struct nk_console_property_data {
    int min_int;
    int max_int;
    int step_int;
    float min_float;
    float max_float;
    float step_float;
    float inc_per_pixel;
    int* val_int;
    float* val_float;
} nk_console_property_data;

typedef struct nk_console_progress_data {
    nk_size max_size;
    nk_size* value_size;
} nk_console_progress_data;

typedef struct nk_console_checkbox_data {
    nk_bool* value_bool;
} nk_console_checkbox_data;

typedef struct nk_console_button_data {
    enum nk_symbol_type symbol;
    void (*onclick)(struct nk_console*);
    int text_length;
} nk_console_button_data;

typedef struct nk_console {
    nk_console_widget_type type;
    const char* text;
    int alignment;

    nk_bool selectable; /** Whether or not the widget can be selected. */
    nk_bool disabled; /** Whether or not the widget is currently disabled. */
    int columns; /** When set, will determine how many dynamic columns to set to for the active row. */
    const char* tooltip; /** Tooltip */

    nk_console_combobox_data combobox;
    nk_console_button_data button;
    nk_console_property_data property;
    nk_console_progress_data progress;
    nk_console_checkbox_data checkbox;

    struct nk_console* parent;
    struct nk_context* context;
    struct nk_console** children;
    struct nk_console* activeParent;
    struct nk_console* activeWidget;
    nk_bool input_processed;

    // Events
    void (*onchange)(struct nk_console*); /** Invoked when there is a change in the value for the widget. */
    struct nk_rect (*render)(struct nk_console*); /** Render the widget. */
} nk_console;

// Console
NK_API nk_console* nk_console_init(struct nk_context* context);
NK_API void nk_console_free(nk_console* console);
NK_API void nk_console_render(nk_console* console);

// Utilities
NK_API nk_console* nk_console_get_top(nk_console* widget);
NK_API int nk_console_get_widget_index(nk_console* widget);
NK_API void nk_console_tooltip(nk_console* console, struct nk_rect widget_bounds);
NK_API void nk_console_check_up_down(nk_console* widget, struct nk_rect bounds);
NK_API nk_bool nk_console_is_active_widget(nk_console* widget);
NK_API void nk_console_set_active_parent(nk_console* new_parent);
NK_API void nk_console_set_active_widget(nk_console* widget);
NK_API void* nk_console_malloc(nk_handle unused, void *old, nk_size size);
NK_API void nk_console_mfree(nk_handle unused, void *ptr);

#include "nuklear_console_button.h"
#include "nuklear_console_label.h"
#include "nuklear_console_checkbox.h"
#include "nuklear_console_progress.h"
#include "nuklear_console_combobox.h"
#include "nuklear_console_property.h"

#endif

#ifdef NK_IMPLEMENTATION
#ifndef NK_CONSOLE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_IMPLEMENTATION_ONCE

#ifndef cvector_clib_free
#define cvector_clib_free(ptr) nk_console_mfree((nk_handle) {.id = 0}, ptr)
#endif
#ifndef cvector_clib_malloc
#define cvector_clib_malloc(size) nk_console_malloc((nk_handle) {.ptr = NULL}, NULL, size)
#endif
#ifndef CVECTOR_H
#define CVECTOR_H "vendor/c-vector/cvector.h"
#endif
#include CVECTOR_H

#ifndef NK_CONSOLE_MALLOC
#ifdef NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_CONSOLE_MALLOC nk_malloc
#else
#error "Requires NK_CONSOLE_MALLOC, or NK_INCLUDE_DEFAULT_ALLOCATOR"
#endif
#endif

#ifndef NK_CONSOLE_FREE
#ifdef NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_CONSOLE_FREE nk_mfree
#else
#error "Requires NK_CONSOLE_FREE, or NK_INCLUDE_DEFAULT_ALLOCATOR"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NK_CONSOLE_IMPLEMENTATION
#define NK_CONSOLE_IMPLEMENTATION
#include "nuklear_console_button.h"
#include "nuklear_console_label.h"
#include "nuklear_console_checkbox.h"
#include "nuklear_console_progress.h"
#include "nuklear_console_combobox.h"
#include "nuklear_console_property.h"
#endif

NK_API void nk_console_set_active_widget(nk_console* widget) {
    if (widget == NULL) {
        return;
    }

    nk_console* parent = widget->parent == NULL ? widget : widget->parent;
    parent->activeWidget = widget;
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

    nk_console* parent = widget;
    while (parent->parent != NULL) {
        parent = parent->parent;
    }

    return parent;
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
    struct nk_rect content_region = nk_window_get_content_region(widget->context);
    nk_uint offsetx, offsety;
    nk_window_get_scroll(widget->context, &offsetx, &offsety);
    if (bounds.y + bounds.h > content_region.y + content_region.h + offsety) {
        nk_uint dest = bounds.y + bounds.h - content_region.y - content_region.h;
        nk_window_set_scroll(widget->context, offsetx, dest);
    }
    else if (bounds.y < content_region.y + offsety) {
        nk_uint dest = bounds.y - content_region.y;
        nk_window_set_scroll(widget->context, offsetx, dest);
    }

    // Only process an active input once.
    if (top->input_processed == nk_false) {
        // Up
        if (nk_input_is_key_pressed(&widget->context->input, NK_KEY_UP)) {
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
        else if (nk_input_is_key_pressed(&widget->context->input, NK_KEY_DOWN)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            while (++widgetIndex < cvector_size(widget->parent->children)) {
                nk_console* target = widget->parent->children[widgetIndex];
                if (target != NULL && target->selectable && !target->disabled) {
                    nk_console_set_active_widget(target);
                    break;
                }
            }
            top->input_processed = nk_true;
        }
        // Back
        else if (nk_input_is_key_pressed(&widget->context->input, NK_KEY_BACKSPACE)) {
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
NK_API nk_console* nk_console_find_first_selectable(nk_console* parent) {
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
NK_API void nk_console_tooltip_display(struct nk_context *ctx, const char *text) {
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

NK_API void nk_console_tooltip(nk_console* console, struct nk_rect widget_bounds) {
    if (console == NULL) {
        return;
    }

    if (console->tooltip != NULL) {
        nk_console_tooltip_display(console->context, console->tooltip);
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

    nk_console* top = nk_console_get_top(console);
    struct nk_rect widget_bounds = console->render != NULL ? console->render(console) : nk_rect(0, 0, 0, 0);

    switch (console->type) {
        case NK_CONSOLE_PARENT: {
            if (console->children != NULL) {
                size_t i;
                for (i = 0; i < cvector_size(console->children); ++i) {
                    nk_console_render(console->children[i]);
                }
            }
        }
        break;
    }

    // Allow mouse to switch focus between active widgets
    if (top->input_processed == nk_false && widget_bounds.w > 0 && nk_input_is_mouse_moved(&console->context->input) && nk_input_is_mouse_hovering_rect(&console->context->input, widget_bounds)) {
        nk_console_set_active_widget(console);
        top->input_processed = nk_true;
    }
}

NK_API void* nk_console_malloc(nk_handle unused, void *old, nk_size size) {
    return NK_CONSOLE_MALLOC(unused, old, size);
}

NK_API void nk_console_mfree(nk_handle unused, void *ptr) {
    NK_CONSOLE_FREE(unused, ptr);
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
    console->context = context;
    console->alignment = NK_TEXT_ALIGN_CENTERED;
    return console;
}

/**
 * Free the given nk_console's data.
 */
NK_API void nk_console_free(nk_console* console) {
    if (console == NULL) {
        return;
    }

    // Clear all the children
    if (console->children != NULL) {
		size_t i;
		for (i = 0; i < cvector_size(console->children); ++i) {
            nk_console_free(console->children[i]);
		}
    }
    cvector_free(console->children);

    nk_handle handle;
    nk_console_mfree(handle, console);
}

#ifdef __cplusplus
}
#endif

#endif
#endif
