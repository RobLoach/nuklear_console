#ifndef NK_CONSOLE_H__
#define NK_CONSOLE_H__

struct nk_console;
struct nk_console_event_handler;

/**
 * The type of events that can be triggered.
 *
 * @see nk_console_add_event()
 */
typedef enum {
    NK_CONSOLE_EVENT_DESTROYED, /** Triggered when the widget is destroyed. */
    NK_CONSOLE_EVENT_CHANGED, /** Triggered when the value for the widget is changed. */
    NK_CONSOLE_EVENT_CLICKED, /** Triggered when the widget is clicked. */
    NK_CONSOLE_EVENT_POST_RENDER_ONCE, /** Triggered after all the widgets have rendered, and the event is removed. */
    NK_CONSOLE_EVENT_PRE_PARENT_RENDER, /** Triggered before the parent widget is rendered. */
} nk_console_event_type;

/**
 * Event handler for a console widget.
 *
 * @param widget The widget that was acted upon.
 * @param user_data Custom user data that is associated with the event.
 *
 * @see nk_console_add_event()
 * @see nk_console_add_event_handler()
 * @see nk_console_trigger_event()
 */
typedef void (*nk_console_event)(struct nk_console* widget, void* user_data);

typedef struct nk_console_event_handler {
    nk_console_event_type type; /** The type of event this handler is for. */
    nk_console_event callback; /** Invoked when this handler is triggered. */
    void* user_data; /** Passed to both 'callback' and 'destructor'. */
    nk_console_event destructor; /** Invoked when this handler is destroyed or overwritten. */
} nk_console_event_handler;

/**
 * A callback to render the given widget.
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
    NK_CONSOLE_TEXTEDIT_TEXT,
    NK_CONSOLE_FILE,
    NK_CONSOLE_IMAGE,
    NK_CONSOLE_SPACING,
    NK_CONSOLE_COLOR,
    NK_CONSOLE_INPUT,
    NK_CONSOLE_INPUT_ACTIVE,
    NK_CONSOLE_RADIO,
    NK_CONSOLE_KNOB_INT,
    NK_CONSOLE_KNOB_FLOAT,
} nk_console_widget_type;

typedef struct nk_console_message {
    char text[256];
    float duration;
} nk_console_message;

typedef struct nk_console {
    nk_console_widget_type type;
    const char* label;
    int label_length;
    nk_flags alignment;

    nk_bool selectable; /** Whether or not the widget can be selected. */
    nk_bool disabled; /** Whether or not the widget is currently disabled. */
    nk_bool visible; /** When false, the widget will not be displayed. */
    int columns; /** When set, will determine how many dynamic columns to set to for the active row. */
    int height; /** When set, will determine the height of the row. */
    const char* tooltip; /** Tooltip */
    void* data; /** Widget-specific data */

    struct nk_console* parent;
    struct nk_context* ctx;
    struct nk_console** children;
    struct nk_console* activeWidget;

    // Events
    nk_console_event_handler* events; /** Events handled for the widget. */
    nk_console_render_event render; /** Render the widget. */
} nk_console;

typedef struct nk_console_top_data {
    nk_console* active_parent; /** The parent that is currently being displayed. */
    nk_bool input_processed; /** Whether or not user input has been processed. */
    nk_bool scroll_requested; /** True if we've switched active widget and need to check scrolling */

    /**
     * Message queue that is to be shown.
     */
    struct nk_console_message* messages;

    /**
     * When set, will determine where messages should appear on the screen.
     */
    struct nk_rect message_bounds;

    /**
     * The gamepad system to use for gamepad input.
     *
     * @see nk_console_get_gamepads()
     * @see nk_console_set_gamepads()
     */
    struct nk_gamepads* gamepads;

    /**
     * Custom user data. This is only applied to the top-level console.
     *
     * @see nk_console_user_data()
     * @see nk_console_set_user_data()
     */
    void* user_data;
} nk_console_top_data;

#if defined(__cplusplus)
extern "C" {
#endif

// Console
NK_API nk_console* nk_console_init(struct nk_context* context);
NK_API void nk_console_free(nk_console* console);
NK_API void nk_console_render(nk_console* console);
NK_API void nk_console_render_window(nk_console* console, const char* title, struct nk_rect bounds, nk_uint flags);

// Utilities
NK_API nk_console* nk_console_get_top(nk_console* widget);
NK_API int nk_console_get_widget_index(nk_console* widget);
NK_API void nk_console_check_tooltip(nk_console* console);
NK_API void nk_console_check_up_down(nk_console* widget, struct nk_rect bounds);
NK_API nk_bool nk_console_is_active_widget(nk_console* widget);
NK_API nk_console* nk_console_active_parent(nk_console* console);
NK_API void nk_console_set_active_parent(nk_console* new_parent);
NK_API void nk_console_set_active_widget(nk_console* widget);
NK_API nk_console* nk_console_get_active_widget(nk_console* widget);
NK_API void* nk_console_malloc(nk_handle unused, void* old, nk_size size);
NK_API void nk_console_mfree(nk_handle unused, void* ptr);
NK_API nk_bool nk_console_button_pushed(nk_console* console, int button);
NK_API void nk_console_set_gamepads(nk_console* console, struct nk_gamepads* gamepads);
NK_API struct nk_gamepads* nk_console_get_gamepads(nk_console* console);
NK_API void nk_console_set_tooltip(nk_console* widget, const char* tooltip);
NK_API void nk_console_set_label(nk_console* widget, const char* label, int label_length);
NK_API const char* nk_console_get_label(nk_console* widget);
NK_API void nk_console_free_children(nk_console* console);
NK_API void nk_console_layout_widget(nk_console* widget);
NK_API void nk_console_add_child(nk_console* parent, nk_console* child);
NK_API void nk_console_set_height(nk_console* widget, int height);
NK_API int nk_console_height(nk_console* widget);
NK_API nk_bool nk_console_selectable(nk_console* widget);

NK_API nk_bool nk_console_trigger_event(nk_console* widget, nk_console_event_type type);
NK_API void nk_console_add_event(nk_console* widget, nk_console_event_type type, nk_console_event callback);
NK_API void nk_console_add_event_handler(nk_console* widget, nk_console_event_type type, nk_console_event callback, void* user_data, nk_console_event destructor);
NK_API void nk_console_event_handler_destroy(nk_console* widget, nk_console_event_handler* handler);

// Backwards compatibility
#define nk_console_onchange(widget) nk_console_trigger_event(widget, NK_CONSOLE_EVENT_CHANGED)
#define nk_console_button_set_onclick(button, onclick) nk_console_add_event(button, NK_CONSOLE_EVENT_CLICKED, onclick)

/**
 * Get the user data for the top-level console.
 *
 * @param console Any widget within the console family.
 *
 * @return The custom user data.
 */
NK_API void* nk_console_user_data(nk_console* console);

/**
 * Set the user data for the top-level console.
 *
 * @param console Any widget within the console family.
 * @param user_data The custom user data to set.
 */
NK_API void nk_console_set_user_data(nk_console* console, void* user_data);

#if defined(__cplusplus)
}
#endif

#define NK_CONSOLE_HEADER_ONLY
#include "nuklear_console_button.h"
#include "nuklear_console_checkbox.h"
#include "nuklear_console_color.h"
#include "nuklear_console_combobox.h"
#include "nuklear_console_file.h"
#include "nuklear_console_file_system.h"
#include "nuklear_console_image.h"
#include "nuklear_console_input.h"
#include "nuklear_console_knob.h"
#include "nuklear_console_label.h"
#include "nuklear_console_message.h"
#include "nuklear_console_progress.h"
#include "nuklear_console_property.h"
#include "nuklear_console_radio.h"
#include "nuklear_console_row.h"
#include "nuklear_console_spacing.h"
#include "nuklear_console_textedit.h"
#include "nuklear_console_textedit_text.h"
#undef NK_CONSOLE_HEADER_ONLY

#if defined(__cplusplus)
#include "nuklear_console.hpp"
#endif

#endif // NK_CONSOLE_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
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
#define cvector_clib_free(ptr) nk_console_mfree(nk_handle_id(0), ptr)
#endif
#ifndef cvector_clib_malloc
#define cvector_clib_malloc(size) nk_console_malloc(nk_handle_id(0), NULL, size)
#endif
#ifndef cvector_clib_calloc
#define cvector_clib_calloc(count, size) NK_ASSERT(0 && "cvector_clib_calloc is not supported")
#endif
#ifndef cvector_clib_realloc
// TODO: Implement our own realloc() using Nuklear's allocator.
#include <stdlib.h>
#define cvector_clib_realloc(ptr, size) realloc(ptr, size)
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

#ifdef __cplusplus
extern "C" {
#endif

NK_API nk_bool nk_input_is_mouse_moved(const struct nk_input* input);

#include "nuklear_console_button.h"
#include "nuklear_console_checkbox.h"
#include "nuklear_console_color.h"
#include "nuklear_console_combobox.h"
#include "nuklear_console_file.h"
#include "nuklear_console_file_system.h"
#include "nuklear_console_image.h"
#include "nuklear_console_input.h"
#include "nuklear_console_knob.h"
#include "nuklear_console_label.h"
#include "nuklear_console_message.h"
#include "nuklear_console_progress.h"
#include "nuklear_console_property.h"
#include "nuklear_console_radio.h"
#include "nuklear_console_row.h"
#include "nuklear_console_spacing.h"
#include "nuklear_console_textedit.h"
#include "nuklear_console_textedit_text.h"

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

NK_API void nk_console_event_handler_destroy(nk_console* widget, nk_console_event_handler* handler) {
    if (handler->destructor) {
        handler->destructor(widget, handler->user_data);
        handler->destructor = NULL;
    }
    handler->callback = NULL;
}

NK_API inline void nk_console_add_event(nk_console* widget, nk_console_event_type type, nk_console_event callback) {
    nk_console_add_event_handler(widget, type, callback, NULL, NULL);
}

NK_API void nk_console_add_event_handler(nk_console* widget, nk_console_event_type type, nk_console_event callback, void* user_data, nk_console_event destructor) {
    if (widget == NULL) {
        return;
    }

    // Manage the top level events.
    if (type == NK_CONSOLE_EVENT_POST_RENDER_ONCE) {
        nk_console_event_handler handler = (nk_console_event_handler){
            .type = type,
            .callback = callback,
            .user_data = (void*)widget,
            .destructor = destructor,
        };
        cvector_push_back(nk_console_get_top(widget)->events, handler);
        return;
    }

    nk_console_event_handler handler = (nk_console_event_handler){
        .type = type,
        .callback = callback,
        .user_data = user_data,
        .destructor = destructor,
    };
    cvector_push_back(widget->events, handler);
}

/**
 * Trigger an event for the given widget.
 *
 * @param widget The widget to trigger the event on.
 * @param type The type of event to trigger.
 *
 * @return nk_true if an event was triggered, nk_false otherwise.
 */
NK_API nk_bool nk_console_trigger_event(nk_console* widget, nk_console_event_type type) {
    if (widget == NULL || widget->events == NULL) {
        return nk_false;
    }

    nk_bool invoked = nk_false;
    size_t count = cvector_size(widget->events);
    for (size_t i = 0; i < count; i++) {
        if (widget->events[i].type == type && widget->events[i].callback) {
            widget->events[i].callback(widget, widget->events[i].user_data);
            invoked = nk_true;
        }
    }

    return invoked;
}

NK_API void nk_console_set_active_widget(nk_console* widget) {
    if (widget == NULL) {
        return;
    }

    nk_console* parent = widget->parent == NULL ? widget : widget->parent;
    parent->activeWidget = widget;
}

/**
 * Retrieves the given widget's parent's active widget.
 */
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

nk_console* nk_console_active_parent(nk_console* console) {
    if (console == NULL) {
        return NULL;
    }

    nk_console* top = nk_console_get_top(console);
    nk_console* active_parent = ((nk_console_top_data*)top->data)->active_parent;
    return active_parent == NULL ? top : active_parent;
}

NK_API void nk_console_set_active_parent(nk_console* new_parent) {
    if (new_parent == NULL) {
        return;
    }

    nk_console* top = nk_console_get_top(new_parent);
    if (top == NULL) {
        return;
    }

    // When switching parents, bring the window scroll to the top to that the window doesn't appear empty.
    nk_window_set_scroll(top->ctx, 0, 0);

    nk_console_top_data* data = (nk_console_top_data*)top->data;
    data->active_parent = new_parent;
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
    widget->height = height < 0 ? 0 : height;
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

    int count = (int)cvector_size(parent->children);
    for (int i = 0; i < count; i++) {
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
    nk_console_top_data* data = (nk_console_top_data*)top->data;

    // Scroll to the active widget if needed.
    if (data->scroll_requested) {
        struct nk_rect content_region = nk_window_get_content_region(widget->ctx);

        nk_uint offsetx, offsety;
        nk_window_get_scroll(widget->ctx, &offsetx, &offsety);
        if (bounds.y + bounds.h > content_region.y + content_region.h + (float)offsety) {
            nk_window_set_scroll(widget->ctx, offsetx, (nk_uint)(bounds.y + bounds.h - content_region.y - content_region.h));
        }
        else if (bounds.y < content_region.y + (float)offsety) {
            nk_window_set_scroll(widget->ctx, offsetx, (nk_uint)(bounds.y - content_region.y));
        }

        data->scroll_requested = nk_false;
    }

    // Only process an active input once.
    if (data->input_processed == nk_false) {
        // Page Up
        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LB)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            int count = 0;
            while (--widgetIndex >= 0) {
                nk_console* target = widget->parent->children[widgetIndex];
                if (target != NULL && nk_console_selectable(target)) {
                    nk_console_set_active_widget(target);
                    data->scroll_requested = nk_true;
                    if (++count > 4) {
                        break;
                    }
                }
            }
            data->input_processed = nk_true;
        }
        // Page Down
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RB)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            int count = 0;
            while (++widgetIndex < (int)cvector_size(widget->parent->children)) {
                nk_console* target = widget->parent->children[widgetIndex];
                if (nk_console_selectable(target)) {
                    nk_console_set_active_widget(target);
                    data->scroll_requested = nk_true;
                    if (++count > 4) {
                        break;
                    }
                }
            }
            data->input_processed = nk_true;
        }
        // Up
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_UP)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            while (--widgetIndex >= 0) {
                nk_console* target = widget->parent->children[widgetIndex];
                if (nk_console_selectable(target)) {
                    nk_console_set_active_widget(target);
                    data->scroll_requested = nk_true;
                    break;
                }
            }
            data->input_processed = nk_true;
        }
        // Down
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_DOWN)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            while (++widgetIndex < (int)cvector_size(widget->parent->children)) {
                nk_console* target = widget->parent->children[widgetIndex];
                if (nk_console_selectable(target)) {
                    nk_console_set_active_widget(target);
                    data->scroll_requested = nk_true;
                    break;
                }
            }
            data->input_processed = nk_true;
        }
        // Back
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_B)) {
            if (nk_console_active_parent(top) == NULL) {
                return;
            }

            if (widget->parent != NULL) {
                if (widget->parent == top) {
                    nk_console_set_active_parent(top);
                }
                else if (widget->parent->parent != NULL) {
                    nk_console_set_active_parent(widget->parent->parent);
                }
                data->scroll_requested = nk_true;
            }

            data->input_processed = nk_true;
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
            if (nk_console_selectable(parent->children[i])) {
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

NK_API nk_bool nk_console_selectable(nk_console* widget) {
    if (widget == NULL) {
        return nk_false;
    }

    return widget->selectable && widget->visible && !widget->disabled;
}

/**
 * Display a tooltip with the given text.
 *
 * @see nk_tooltip()
 * @todo Support multiline tooltips with nk_text_calculate_text_bounds()
 */
static void nk_console_tooltip_display(struct nk_context* ctx, const char* text) {
    const struct nk_style* style;
    struct nk_vec2 padding;

    style = &ctx->style;
    padding = style->window.padding;

    float text_height = (style->font->height + padding.y);
    float x = ctx->input.mouse.pos.x;
    float y = ctx->input.mouse.pos.y;

    // Display the tooltip at the bottom of the window, manipulating the mouse position
    struct nk_rect windowbounds = nk_window_get_bounds(ctx);
    ctx->input.mouse.pos.x = windowbounds.x;
    ctx->input.mouse.pos.y = windowbounds.y + windowbounds.h - text_height - padding.y * 2.0f;

    if (nk_tooltip_begin(ctx, windowbounds.w)) {
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
    if (console == NULL || console->visible == nk_false) {
        return;
    }

    // First run on this game loop
    if (console->parent == NULL) {
        nk_console_top_data* data = (nk_console_top_data*)console->data;

        // Reset the input state.
        data->input_processed = nk_false;

        nk_console_trigger_event(data->active_parent, NK_CONSOLE_EVENT_PRE_PARENT_RENDER);

        // Render the active message.
        nk_console_render_message(console);

        // Render all of the parent's children.
        if (data->active_parent->children != NULL) {
            // Make sure there's an active widget selected.
            if (data->active_parent->activeWidget == NULL) {
                nk_console_set_active_widget(nk_console_find_first_selectable(data->active_parent));
            }
            else {
                // Make sure the widget actually exists and can be selected.
                nk_bool widgetFound = nk_false;
                for (size_t i = 0; i < cvector_size(data->active_parent->children); ++i) {
                    if (data->active_parent->children[i] == data->active_parent->activeWidget) {
                        // Ensure the widget is still selectable.
                        if (nk_console_selectable(data->active_parent->activeWidget)) {
                            widgetFound = nk_true;
                        }
                        break;
                    }
                }
                if (widgetFound == nk_false) {
                    nk_console_set_active_widget(nk_console_find_first_selectable(data->active_parent));
                }
            }

            // Render all the children
            for (size_t i = 0; i < cvector_size(data->active_parent->children); ++i) {
                nk_console_render(data->active_parent->children[i]);
            }
        }

        // Invoke the post-render events.
        size_t count = cvector_size(console->events);
        if (count > 0) {
            nk_bool can_erase = nk_true;
            for (size_t i = 0; i < count; i++) {
                if (console->events[i].type == NK_CONSOLE_EVENT_POST_RENDER_ONCE) {
                    // Call the callback if it's set.
                    if (console->events[i].callback != NULL) {
                        console->events[i].callback((nk_console*)console->events[i].user_data, NULL);
                        console->events[i].callback = NULL;
                    }
                }
                else {
                    can_erase = nk_false;
                }
            }
            if (can_erase) {
                cvector_clear(console->events);
            }
        }
        return;
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
        nk_console_top_data* data = (nk_console_top_data*)top->data;
        if (data->input_processed == nk_false && nk_input_is_mouse_hovering_rect(&console->ctx->input, widget_bounds)) {
            // Select the widget, if possible.
            if (nk_console_selectable(console)) {
                nk_console_set_active_widget(console);
                data->input_processed = nk_true;
            }
        }
    }
}

NK_API void* nk_console_malloc(nk_handle unused, void* old, nk_size size) {
    NK_UNUSED(unused);
    NK_UNUSED(old);
    return NK_CONSOLE_MALLOC(unused, old, size);
}

NK_API void nk_console_mfree(nk_handle unused, void* ptr) {
    NK_UNUSED(unused);
    NK_CONSOLE_FREE(unused, ptr);
}

/**
 * Initialize a new nk_console.
 *
 * @param context The associated Nuklear context.
 */
NK_API nk_console* nk_console_init(struct nk_context* context) {
    nk_handle handle;
    nk_console* console = (nk_console*)nk_console_malloc(handle, NULL, sizeof(nk_console));
    nk_zero(console, sizeof(nk_console));
    console->type = NK_CONSOLE_PARENT;
    console->ctx = context;
    console->alignment = NK_TEXT_ALIGN_CENTERED;
    console->visible = nk_true;

    nk_console_top_data* data = (nk_console_top_data*)nk_console_malloc(handle, NULL, sizeof(nk_console_top_data));
    nk_zero(data, sizeof(nk_console_top_data));
    data->active_parent = console;
    console->data = data;

    return console;
}

/**
 * Renders the given console to a new window with the given properties.
 *
 * @param console The console to render.
 * @param title The title of the window.
 * @param bounds The bounds of the window.
 * @param flags The flags to apply to the window.
 */
NK_API void nk_console_render_window(nk_console* console, const char* title, struct nk_rect bounds, nk_uint flags) {
    if (console == NULL) {
        return;
    }

    if (nk_begin(console->ctx, title, bounds, flags)) {
        nk_console_render(console);
    }

    nk_end(console->ctx);
}

/**
 * Free the given nk_console's data.
 */
NK_API void nk_console_free(nk_console* console) {
    if (console == NULL) {
        return;
    }
    nk_handle handle = {0};

    // Clean up the events
    if (console->events != NULL) {
        nk_console_trigger_event(console, NK_CONSOLE_EVENT_DESTROYED);

        // Destroy all event handlers.
        for (size_t i = 0; i < cvector_size(console->events); ++i) {
            nk_console_event_handler_destroy(console, &console->events[i]);
        }

        cvector_free(console->events);
        console->events = NULL;
    }

    // Clear any component-specific data.
    if (console->data != NULL) {
        // Free the top data.
        if (console->type == NK_CONSOLE_PARENT) {
            nk_console_top_data* data = (nk_console_top_data*)console->data;
            if (data->messages != NULL) {
                cvector_free(data->messages);
                data->messages = NULL;
            }
        }

        nk_console_mfree(handle, console->data);
        console->data = NULL;
    }

    // Clear all the children
    nk_console_free_children(console);

    nk_console_mfree(handle, console);
}

NK_API void* nk_console_user_data(nk_console* console) {
    nk_console* top = nk_console_get_top(console);
    if (top == NULL) {
        return NULL;
    }

    return ((nk_console_top_data*)top->data)->user_data;
}

NK_API void nk_console_set_user_data(nk_console* console, void* user_data) {
    nk_console* top = nk_console_get_top(console);
    if (top == NULL) {
        return;
    }

    nk_console_top_data* data = (nk_console_top_data*)top->data;
    data->user_data = user_data;
}

NK_API void nk_console_free_children(nk_console* console) {
    if (console == NULL || console->children == NULL) {
        return;
    }

    // Since there won't be any children, make sure to unselect any active child.
    console->activeWidget = NULL;

    // Clear them all.
    size_t count = cvector_size(console->children);
    for (size_t i = 0; i < count; i++) {
        nk_console_free(console->children[i]);
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
    nk_layout_row_dynamic(widget->ctx, (float)widget->height, widget->columns);
}

NK_API void nk_console_set_gamepads(nk_console* console, struct nk_gamepads* gamepads) {
    nk_console* top = nk_console_get_top(console);
    if (top == NULL) {
        return;
    }

    nk_console_top_data* data = (nk_console_top_data*)top->data;
    if (data == NULL) {
        return;
    }

    data->gamepads = gamepads;
}

NK_API struct nk_gamepads* nk_console_get_gamepads(nk_console* console) {
    nk_console* top = nk_console_get_top(console);
    if (top == NULL) {
        return NULL;
    }

    nk_console_top_data* data = (nk_console_top_data*)top->data;
    if (data == NULL) {
        return NULL;
    }
    return data->gamepads;
}

NK_API nk_bool nk_console_button_pushed(nk_console* console, int button) {
    if (console == NULL) {
        return nk_false;
    }

    // Get the top console.
    if (console->parent != NULL) {
        console = nk_console_get_top(console);
    }

    // Gamepad
    nk_console_top_data* data = (nk_console_top_data*)console->data;
    if (nk_gamepad_is_button_pressed(data->gamepads, -1, (enum nk_gamepad_button)button)) {
        return nk_true;
    }

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
        case NK_GAMEPAD_BUTTON_BACK:
            return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_SHIFT);
            // case NK_GAMEPAD_BUTTON_START: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_UP);
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
