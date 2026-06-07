#ifndef NK_CONSOLE_H__
#define NK_CONSOLE_H__

#include "nuklear_console_gamepad_stub.h"

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
    NK_CONSOLE_EVENT_FOCUS, /** Triggered when the widget gains focus/becomes the active widget. */
    NK_CONSOLE_EVENT_BLUR, /** Triggered when the widget loses focus/is no longer the active widget. */
    NK_CONSOLE_EVENT_BACK, /** Triggered on the active parent widget when the user navigates back. */
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
    NK_CONSOLE_RULE_HORIZONTAL,
    NK_CONSOLE_TREE,
    NK_CONSOLE_LIST_VIEW,
} nk_console_widget_type;

#ifndef NK_CONSOLE_MESSAGE_MAX_LENGTH
/**
 * Maximum number of characters (excluding null terminator) for a message.
 * Define before including nuklear_console.h to allow longer messages.
 */
#define NK_CONSOLE_MESSAGE_MAX_LENGTH 255
#endif // NK_CONSOLE_MESSAGE_MAX_LENGTH

typedef struct nk_console_message {
    char text[NK_CONSOLE_MESSAGE_MAX_LENGTH + 1];
    float duration;
    float scroll_x;
} nk_console_message;

/**
 * Indicates which screen edge a message slides in from.
 *
 * @see nk_console_set_message_position()
 * @see nk_console_get_message_position()
 */
typedef enum {
    NK_CONSOLE_MESSAGE_POSITION_BOTTOM = 0, /** Slide in from the bottom of the screen (default). */
    NK_CONSOLE_MESSAGE_POSITION_TOP, /** Slide in from the top edge. */
    NK_CONSOLE_MESSAGE_POSITION_LEFT, /** Slide in from the left edge. */
    NK_CONSOLE_MESSAGE_POSITION_RIGHT, /** Slide in from the right edge. */
} nk_console_message_position;

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
    nk_bool scrollbar_required; /** True when the scrollbar is needed to be rendered. @see nk_console_render_window() */
    nk_console* scroll_to_widget; /** When set by nk_console_navigate_back, the render loop scrolls the window to this widget's bounds. */

    /**
     * Message queue that is to be shown.
     */
    struct nk_console_message* messages;

    /**
     * When set, will determine where messages should appear on the screen.
     */
    struct nk_rect message_bounds;

    /**
     * Which screen edge messages slide in from.
     *
     * @see nk_console_set_message_position()
     * @see nk_console_get_message_position()
     */
    nk_console_message_position message_position;

    /**
     * The gamepad system to use for gamepad input.
     *
     * @see nk_console_get_gamepads()
     * @see nk_console_set_gamepads()
     */
    struct nk_gamepads* gamepads;

    /**
     * Which gamepad number to use for input. -1 means any/all gamepads.
     *
     * @see nk_console_set_gamepad_num()
     * @see nk_console_get_gamepad_num()
     */
    int gamepad_num;

    /**
     * Custom user data. This is only applied to the top-level console.
     *
     * @see nk_console_user_data()
     * @see nk_console_set_user_data()
     */
    void* user_data;

    float up_down_hold_timer; /** Total time up/down has been held continuously, used for acceleration. */
    float up_down_repeat_timer; /** Time accumulator since the last repeated up/down move. */

    /** State for one analog axis navigation channel (up/down or left/right). */
    struct nk_console_axis_channel {
        float timer; /** Repeat accumulator. -1 when inactive. */
        float hold_timer; /** Total active hold time, drives acceleration. */
    } axis_ud, axis_lr;

    nk_bool axis_up_fired; /** True this frame if an axis fired an up event. */
    nk_bool axis_down_fired; /** True this frame if an axis fired a down event. */
    nk_bool axis_left_fired; /** True this frame if an axis fired a left event. */
    nk_bool axis_right_fired; /** True this frame if an axis fired a right event. */

    nk_bool drag_scroll_active; /** True when a drag-scroll gesture is in progress. */
    nk_bool drag_scroll_initiated; /** True when the drag started from the content area (not scrollbar). */
    struct nk_vec2 drag_scroll_origin; /** Mouse position when the drag started. */
    nk_uint drag_scroll_start_x; /** Window scroll X at drag start. */
    nk_uint drag_scroll_start_y; /** Window scroll Y at drag start. */
    nk_uint drag_scroll_max_x; /** Maximum scroll X, updated each frame after render. */
    nk_uint drag_scroll_max_y; /** Maximum scroll Y, updated each frame after render. */

    float tooltip_scroll_x; /** Horizontal marquee offset for the active tooltip. */
    const char* tooltip_last; /** Pointer to the last tooltip shown; used to detect tooltip changes and reset scroll. */
} nk_console_top_data;

#if defined(__cplusplus)
extern "C" {
#endif

/** @defgroup console Console lifecycle */
/** Initialize a top-level console for the given Nuklear context. @return The root console widget. */
NK_API nk_console* nk_console_init(struct nk_context* context);
/** Free the console and all its children. */
NK_API void nk_console_free(nk_console* console);
/** Render the console inside the current Nuklear window. */
NK_API void nk_console_render(nk_console* console);
/** Render the console inside a new Nuklear window. @return The bounding rect of the rendered window. */
NK_API struct nk_rect nk_console_render_window(nk_console* console, const char* title, struct nk_rect bounds, nk_uint flags);

/** @defgroup utilities Widget utilities */
/** Return the top-level console that owns @p widget. */
NK_API nk_console* nk_console_get_top(nk_console* widget);
/** Return the zero-based index of @p widget among its siblings. */
NK_API int nk_console_get_widget_index(nk_console* widget);
/** Draw the tooltip for the currently active widget, if any. */
NK_API void nk_console_check_tooltip(nk_console* console);
/** Handle keyboard/gamepad up and down navigation for @p widget. */
NK_API void nk_console_check_up_down(nk_console* widget);
/** Return nk_true if @p widget is the currently focused widget. */
NK_API nk_bool nk_console_is_active_widget(nk_console* widget);
/** Return the currently active parent (the visible menu level). */
NK_API nk_console* nk_console_active_parent(nk_console* console);
/** Set the active parent, switching the visible menu level. */
NK_API void nk_console_set_active_parent(nk_console* new_parent);
/** Set the focused widget within the active parent. */
NK_API void nk_console_set_active_widget(nk_console* widget);
/** Return the currently focused widget, or NULL if none. */
NK_API nk_console* nk_console_get_active_widget(nk_console* widget);
/** Allocate memory using the console's allocator (compatible with nk_allocator). */
NK_API void* nk_console_malloc(nk_handle unused, void* old, nk_size size);
/** Free memory using the console's allocator (compatible with nk_allocator). */
NK_API void nk_console_mfree(nk_handle unused, void* ptr);
/** Return nk_true if @p button was just pressed this frame (rising edge). */
NK_API nk_bool nk_console_button_pushed(nk_console* console, int button);
/** Return nk_true if @p button is currently held down. */
NK_API nk_bool nk_console_button_down(nk_console* console, int button);
/** Attach a nk_gamepads context to the console for gamepad input. */
NK_API void nk_console_set_gamepads(nk_console* console, struct nk_gamepads* gamepads);
/** Return the nk_gamepads context attached to the console. */
NK_API struct nk_gamepads* nk_console_get_gamepads(nk_console* console);
/** Set the number of gamepads polled each frame. */
NK_API void nk_console_set_gamepad_num(nk_console* console, int num);
/** Return the number of gamepads polled each frame. */
NK_API int nk_console_get_gamepad_num(nk_console* console);
/** Set the tooltip string shown when @p widget is focused. */
NK_API void nk_console_set_tooltip(nk_console* widget, const char* tooltip);
/** Replace the label of @p widget; pass -1 for @p label_length to auto-detect. */
NK_API void nk_console_set_label(nk_console* widget, const char* label, int label_length);
/** Return the current label of @p widget. */
NK_API const char* nk_console_get_label(nk_console* widget);
/** Destroy and free all child widgets of @p console. */
NK_API void nk_console_free_children(nk_console* console);
/** Emit the Nuklear row layout for @p widget (called internally before rendering). */
NK_API void nk_console_layout_widget(nk_console* widget);
/** Append @p child as a child of @p parent. */
NK_API void nk_console_add_child(nk_console* parent, nk_console* child);
/** Override the row height of @p widget in pixels. */
NK_API void nk_console_set_height(nk_console* widget, int height);
/** Return the row height of @p widget in pixels. */
NK_API int nk_console_height(nk_console* widget);
/** Return nk_true if @p widget can receive focus (is selectable). */
NK_API nk_bool nk_console_selectable(nk_console* widget);

/** Fire all handlers registered for @p type on @p widget. @return nk_true if any handler ran. */
NK_API nk_bool nk_console_trigger_event(nk_console* widget, nk_console_event_type type);
/** Register a simple callback for @p type on @p widget (no user data or destructor). */
NK_API void nk_console_add_event(nk_console* widget, nk_console_event_type type, nk_console_event callback);
/** Register a callback with user data and an optional destructor for @p type on @p widget. */
NK_API void nk_console_add_event_handler(nk_console* widget, nk_console_event_type type, nk_console_event callback, void* user_data, nk_console_event destructor);
/** Unregister and free a specific event handler from @p widget. */
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

/**
 * Navigate back from the given widget to its parent, triggering NK_CONSOLE_EVENT_BACK.
 *
 * The event is triggered after the new parent is selected.
 *
 * @param leaving_parent The active parent widget being navigated away from.
 *
 * @see NK_CONSOLE_EVENT_BACK
 * @see nk_console_button_back()
 */
NK_API void nk_console_navigate_back(nk_console* leaving_parent);

/**
 * Get a widget by a slash-separated path of labels, starting from the top-level console.
 *
 * Example: nk_console_find_by_path(console, "Widgets/Labels")
 * A leading slash is optional: "/Widgets" and "Widgets" are equivalent.
 *
 * @param console Any widget within the console family.
 * @param path Slash-separated widget labels (e.g. "Widgets/Labels").
 *
 * @return The matching widget, or NULL if not found.
 */
NK_API nk_console* nk_console_find_by_path(nk_console* console, const char* path);

/**
 * Navigate to a widget by a slash-separated path of labels.
 *
 * If the target widget has children, it becomes the active parent. If it has no children, its parent becomes the active parent and the widget is selected as the active widget.
 *
 * Example: nk_console_navigate_to_path(console, "Widgets/Labels")
 *
 * @param console Any widget within the console family.
 * @param path Slash-separated widget labels (e.g. "Widgets/Labels").
 *
 * @return nk_true if navigation succeeded, nk_false if the path was not found.
 */
NK_API nk_bool nk_console_navigate_to_path(nk_console* console, const char* path);

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
#include "nuklear_console_list_view.h"
#include "nuklear_console_marquee.h"
#include "nuklear_console_message.h"
#include "nuklear_console_progress.h"
#include "nuklear_console_property.h"
#include "nuklear_console_radio.h"
#include "nuklear_console_row.h"
#include "nuklear_console_rule_horizontal.h"
#include "nuklear_console_spacing.h"
#include "nuklear_console_textedit.h"
#include "nuklear_console_textedit_text.h"
#include "nuklear_console_tree.h"
#undef NK_CONSOLE_HEADER_ONLY

#if defined(__cplusplus)
#include "nuklear_console.hpp"
#endif

#endif // NK_CONSOLE_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_IMPLEMENTATION_ONCE

#ifndef NK_BUTTON_TRIGGER_ON_RELEASE
/**
 * Define NK_BUTTON_TRIGGER_ON_RELEASE prior to nuklear.h.
 *
 * This is required because the input events are triggered incorrectly otherwise, resulting in undesirable behavior.
 *
 * @code
 * #define NK_BUTTON_TRIGGER_ON_RELEASE
 * #define NK_IMPLEMENTATION
 * #include "nuklear.h"
 *
 * #define NK_CONSOLE_IMPLEMENTATION
 * #include "nuklear_console.h"
 * @endcode
 *
 * You are able to ignore this warning by using #define NK_CONSOLE_IGNORE_BUTTON_TRIGGER_ON_RELEASE
 */
#ifndef NK_CONSOLE_IGNORE_BUTTON_TRIGGER_ON_RELEASE
#warning "nuklear_console requires NK_BUTTON_TRIGGER_ON_RELEASE. Add #define NK_BUTTON_TRIGGER_ON_RELEASE prior to #define NK_IMPLEMENTATION ."
#endif
#endif

#ifndef NK_CONSOLE_AXIS_DEADZONE
#define NK_CONSOLE_AXIS_DEADZONE 0.22f
#endif
#ifndef NK_CONSOLE_AXIS_REPEAT_INTERVAL
#define NK_CONSOLE_AXIS_REPEAT_INTERVAL 0.5f
#endif
#ifndef NK_CONSOLE_DRAG_THRESHOLD
#define NK_CONSOLE_DRAG_THRESHOLD 8.0f
#endif

#ifndef NK_CONSOLE_KEY_BACK
/**
 * The Nuklear `enum nk_keys` used to go back in the menu heirarchy.
 *
 * @see nk_console_button_pushed()
 */
#define NK_CONSOLE_KEY_BACK NK_KEY_TEXT_RESET_MODE
#endif // NK_CONSOLE_KEY_BACK

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

// When none of malloc/free/realloc are user-overridden, use a fat-pointer
// scheme so all three route through Nuklear's allocator without stdlib.
// Each allocation is prefixed by an nk_size storing the requested size so
// that realloc can know how many bytes to copy when growing the block.
#if !defined(cvector_clib_malloc) && !defined(cvector_clib_free) && !defined(cvector_clib_realloc)
/**
 * Allocate @p size bytes via NK_CONSOLE_MALLOC, prefixing the block with an
 * nk_size header that records the requested size for use by
 * nk_console_cvector_realloc().
 * @param size Number of bytes to allocate.
 * @return Pointer to the usable region, or NULL on failure.
 */
static void* nk_console_cvector_malloc(nk_size size) {
    nk_size* block = (nk_size*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, size + sizeof(nk_size));
    if (!block) return NULL;
    *block = size;
    return (void*)(block + 1);
}
/**
 * Free a pointer previously returned by nk_console_cvector_malloc() or
 * nk_console_cvector_realloc() via NK_CONSOLE_FREE.
 * @param ptr Pointer to free, or NULL (no-op).
 */
static void nk_console_cvector_free(void* ptr) {
    if (!ptr) return;
    NK_CONSOLE_FREE(nk_handle_id(0), (nk_size*)ptr - 1);
}
/**
 * Resize a block previously returned by nk_console_cvector_malloc().
 * Copies the lesser of the old and new sizes, then frees the old block.
 * @param old_ptr Existing allocation, or NULL to perform a fresh allocation.
 * @param new_size Desired size in bytes.
 * @return Pointer to the resized region, or NULL on allocation failure.
 */
static void* nk_console_cvector_realloc(void* old_ptr, nk_size new_size) {
    void* new_ptr = nk_console_cvector_malloc(new_size);
    if (!new_ptr) return NULL;
    if (old_ptr) {
        nk_size old_size = *((nk_size*)old_ptr - 1);
        NK_MEMCPY(new_ptr, old_ptr, old_size < new_size ? old_size : new_size);
        nk_console_cvector_free(old_ptr);
    }
    return new_ptr;
}
#define cvector_clib_malloc(size)       nk_console_cvector_malloc(size)
#define cvector_clib_free(ptr)          nk_console_cvector_free(ptr)
#define cvector_clib_realloc(ptr, size) nk_console_cvector_realloc(ptr, size)
#else
    #ifndef cvector_clib_free
    #define cvector_clib_free(ptr) nk_console_mfree(nk_handle_id(0), ptr)
    #endif
    #ifndef cvector_clib_malloc
    #define cvector_clib_malloc(size) nk_console_malloc(nk_handle_id(0), NULL, size)
    #endif
    #ifndef cvector_clib_realloc
    #include <stdlib.h>
    #define cvector_clib_realloc(ptr, size) realloc(ptr, size)
    #endif
#endif
#ifndef cvector_clib_calloc
#define cvector_clib_calloc(count, size) NK_ASSERT(0 && "cvector_clib_calloc is not supported")
#endif
#ifndef cvector_clib_assert
#define cvector_clib_assert(expression) NK_ASSERT(expression)
#endif
#ifndef cvector_clib_memcpy
#define cvector_clib_memcpy(dest, src, count) NK_MEMCPY(dest, src, count)
#endif
#ifndef cvector_clib_memmove
/**
 * Move @p count bytes from @p src to @p dest, correctly handling overlapping
 * regions by using a forward copy when dest <= src and a backward copy
 * otherwise (Nuklear does not provide memmove).
 * @param dest Destination buffer.
 * @param src  Source buffer.
 * @param count Number of bytes to move.
 * @return @p dest.
 */
static void* nk_console_cvector_memmove(void* dest, const void* src, nk_size count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    if (d == s || count == 0) return dest;
    if (d < s) {
        NK_MEMCPY(dest, src, count);
    } else {
        d += count; s += count;
        while (count--) *--d = *--s;
    }
    return dest;
}
#define cvector_clib_memmove(dest, src, count) nk_console_cvector_memmove(dest, src, count)
#endif
#ifndef CVECTOR_H
#define CVECTOR_H "vendor/c-vector/cvector.h"
#endif
#include CVECTOR_H


#ifdef __cplusplus
extern "C" {
#endif

#include "nuklear_console_button.h"
#include "nuklear_console_checkbox.h"
#include "nuklear_console_color.h"
#include "nuklear_console_combobox.h"
#include "nuklear_console_file.h"
#include "nuklear_console_gamepad_stub.h"
#include "nuklear_console_file_system.h"
#include "nuklear_console_image.h"
#include "nuklear_console_input.h"
#include "nuklear_console_knob.h"
#include "nuklear_console_label.h"
#include "nuklear_console_list_view.h"
#include "nuklear_console_marquee.h"
#include "nuklear_console_message.h"
#include "nuklear_console_progress.h"
#include "nuklear_console_property.h"
#include "nuklear_console_radio.h"
#include "nuklear_console_row.h"
#include "nuklear_console_rule_horizontal.h"
#include "nuklear_console_spacing.h"
#include "nuklear_console_textedit.h"
#include "nuklear_console_textedit_text.h"
#include "nuklear_console_tree.h"

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
    if (parent->activeWidget != widget) {
        nk_console_trigger_event(parent->activeWidget, NK_CONSOLE_EVENT_BLUR);
        parent->activeWidget = widget;
        nk_console_trigger_event(widget, NK_CONSOLE_EVENT_FOCUS);
    }
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

NK_API nk_console* nk_console_active_parent(nk_console* console) {
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
    // TODO: Fix the scroll on the new window, since it may not be centered on the active widget.
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
NK_API void nk_console_check_up_down(nk_console* widget) {
    nk_console* top = nk_console_get_top(widget);
    nk_console_top_data* data = (nk_console_top_data*)top->data;

    // Handle hold-to-accelerate timers for up/down navigation.
    nk_bool up_held = nk_console_button_down(top, NK_GAMEPAD_BUTTON_UP);
    nk_bool down_held = nk_console_button_down(top, NK_GAMEPAD_BUTTON_DOWN);
    nk_bool up_down_repeat_fire = nk_false;
    if (up_held || down_held) {
        if (top->ctx->delta_time_seconds > 0) {
            data->up_down_hold_timer += top->ctx->delta_time_seconds;
            data->up_down_repeat_timer += top->ctx->delta_time_seconds;
            // 0.045s initial delay. Speeding up gradually
            float interval = NK_MAX(0.045f, 0.5f - data->up_down_hold_timer * 0.125f);
            if (data->up_down_repeat_timer >= interval) {
                up_down_repeat_fire = nk_true;
                data->up_down_repeat_timer -= interval;
            }
        }
    }
    else {
        data->up_down_hold_timer = 0;
        data->up_down_repeat_timer = 0;
    }

    // Only process an active input once.
    if (data->input_processed == nk_false) {
        // Page Up
        if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_LB)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            nk_console* target = NULL;
            int count = 0;
            while (--widgetIndex >= 0) {
                nk_console* t = widget->parent->children[widgetIndex];
                if (t != NULL && nk_console_selectable(t)) {
                    target = t;
                    if (++count > 4) {
                        break;
                    }
                }
            }
            if (target != NULL) {
                nk_console_set_active_widget(target);
                data->scroll_to_widget = target;
            }
            data->input_processed = nk_true;
        }
        // Page Down
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_RB)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            nk_console* target = NULL;
            int count = 0;
            if (widget->parent != NULL) {
                while (++widgetIndex < (int)cvector_size(widget->parent->children)) {
                    nk_console* t = widget->parent->children[widgetIndex];
                    if (t != NULL && nk_console_selectable(t)) {
                        target = t;
                        if (++count > 4) {
                            break;
                        }
                    }
                }
            }
            if (target != NULL) {
                nk_console_set_active_widget(target);
                data->scroll_to_widget = target;
            }
            data->input_processed = nk_true;
        }
        // Up
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_UP) || (up_held && up_down_repeat_fire)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            while (--widgetIndex >= 0) {
                nk_console* target = widget->parent->children[widgetIndex];
                if (target != NULL && nk_console_selectable(target)) {
                    nk_console_set_active_widget(target);
                    data->scroll_to_widget = target;
                    break;
                }
            }
            data->input_processed = nk_true;
        }
        // Down
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_DOWN) || (down_held && up_down_repeat_fire)) {
            int widgetIndex = nk_console_get_widget_index(widget);
            if (widget->parent != NULL) {
                while (++widgetIndex < (int)cvector_size(widget->parent->children)) {
                    nk_console* target = widget->parent->children[widgetIndex];
                    if (target != NULL && nk_console_selectable(target)) {
                        nk_console_set_active_widget(target);
                        data->scroll_to_widget = target;
                        break;
                    }
                }
            }
            data->input_processed = nk_true;
        }
        // Back
        else if (nk_console_button_pushed(top, NK_GAMEPAD_BUTTON_B)) {
            if (nk_console_active_parent(top) == NULL) {
                data->input_processed = nk_true;
                return;
            }

            if (widget->parent != NULL) {
                nk_console_navigate_back(widget->parent);
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
 * Gets whether or not the given widget can be selected.
 */
NK_API nk_bool nk_console_selectable(nk_console* widget) {
    if (widget == NULL) {
        return nk_false;
    }

    return widget->selectable && widget->visible && !widget->disabled;
}

#ifndef NK_CONSOLE_TOOLTIP_SCROLL_SPEED
#define NK_CONSOLE_TOOLTIP_SCROLL_SPEED NK_CONSOLE_MARQUEE_SCROLL_SPEED
#endif
#ifndef NK_CONSOLE_TOOLTIP_SCROLL_PAUSE
#define NK_CONSOLE_TOOLTIP_SCROLL_PAUSE NK_CONSOLE_MARQUEE_SCROLL_PAUSE
#endif

/**
 * Display a tooltip with the given text.
 *
 * @see nk_tooltip()
 */
static void nk_console_tooltip_display(nk_console* console, const char* text) {
    struct nk_context* ctx = console->ctx;
    const struct nk_style* style = &ctx->style;
    float text_height = style->font->height + style->window.padding.y;
    float x = ctx->input.mouse.pos.x;
    float y = ctx->input.mouse.pos.y;

    struct nk_rect windowbounds = nk_window_get_bounds(ctx);
    ctx->input.mouse.pos.x = windowbounds.x;
    ctx->input.mouse.pos.y = windowbounds.y + windowbounds.h - text_height - style->window.padding.y * 2.0f - style->window.border;

    nk_console_top_data* data = (nk_console_top_data*)nk_console_get_top(console)->data;

    // Reset scroll on tooltip change. Relies on pointer identity — works for string
    // literals and persistent pointers; resets every frame for stack-allocated strings.
    if (data->tooltip_last != text) {
        data->tooltip_last = text;
        data->tooltip_scroll_x = 0.0f;
    }

    int text_len = nk_strlen(text);
    float full_text_width = style->font->width(style->font->userdata, style->font->height, text, text_len);
    nk_console_marquee_tooltip_render(ctx, text, text_len, full_text_width,
        windowbounds.w - style->window.border, text_height,
        NK_CONSOLE_TOOLTIP_SCROLL_SPEED, NK_CONSOLE_TOOLTIP_SCROLL_PAUSE,
        &data->tooltip_scroll_x);

    ctx->input.mouse.pos.x = x;
    ctx->input.mouse.pos.y = y;
}

/**
 * Sees if there's an active tooltip, and displays it as needed.
 */
NK_API void nk_console_check_tooltip(nk_console* console) {
    if (console == NULL) {
        return;
    }

    if (console->tooltip != NULL) {
        nk_console_tooltip_display(console, console->tooltip);
    }
}

/**
 * Render the given console widget.
 *
 * @param console The console widget to display.
 */
/**
 * Tick one axis navigation channel. Returns nk_true if a navigation event should fire this frame.
 * @internal
 */
static nk_bool nk_console_axis_tick(struct nk_console_axis_channel* ch, float strength, float delta) {
    if (strength <= 0.0f) {
        ch->timer = -1.0f;
        ch->hold_timer = 0.0f;
        return nk_false;
    }
    if (ch->timer < 0.0f) {
        ch->timer = 0.0f;
        return nk_true; // Fire immediately on first active frame.
    }
    ch->hold_timer += delta;
    ch->timer += delta * strength * (1.0f + NK_MIN(ch->hold_timer, 4.0f) * 0.5f);
    if (ch->timer >= NK_CONSOLE_AXIS_REPEAT_INTERVAL) {
        ch->timer = 0.0f; // Protect against hitting many frames at the same time.
        return nk_true;
    }
    return nk_false;
}

/**
 * Checks all the gamepad input states, and combines all the gamepad axis into one.
 *
 * @internal
 */
static void nk_console_axis_update(nk_console* console) {
    nk_console_top_data* data = (nk_console_top_data*)console->data;
    data->axis_up_fired = data->axis_down_fired = data->axis_left_fired = data->axis_right_fired = nk_false;
    if (data->gamepads == NULL || console->ctx->delta_time_seconds <= 0) {
        return;
    }

    // Grab the axis data.
    float left_y = nk_gamepad_get_axis(data->gamepads, data->gamepad_num, NK_GAMEPAD_AXIS_LEFT_Y);
    float left_x = nk_gamepad_get_axis(data->gamepads, data->gamepad_num, NK_GAMEPAD_AXIS_LEFT_X);
    float right_y = nk_gamepad_get_axis(data->gamepads, data->gamepad_num, NK_GAMEPAD_AXIS_RIGHT_Y);
    float right_x = nk_gamepad_get_axis(data->gamepads, data->gamepad_num, NK_GAMEPAD_AXIS_RIGHT_X);
    float ltrig = nk_gamepad_get_axis(data->gamepads, data->gamepad_num, NK_GAMEPAD_AXIS_LEFT_TRIGGER);
    float rtrig = nk_gamepad_get_axis(data->gamepads, data->gamepad_num, NK_GAMEPAD_AXIS_RIGHT_TRIGGER);

    // Per-direction strengths.
    float up_strength = 0.0f, down_strength = 0.0f;
    float left_strength = 0.0f, right_strength = 0.0f;
    if (left_x < -NK_CONSOLE_AXIS_DEADZONE) left_strength = NK_MAX(left_strength, -left_x * 2.0f);
    if (left_x > NK_CONSOLE_AXIS_DEADZONE) right_strength = NK_MAX(right_strength, left_x * 2.0f);
    if (left_y < -NK_CONSOLE_AXIS_DEADZONE) up_strength = NK_MAX(up_strength, -left_y * 2.0f);
    if (left_y > NK_CONSOLE_AXIS_DEADZONE) down_strength = NK_MAX(down_strength, left_y * 2.0f);
    if (right_x < -NK_CONSOLE_AXIS_DEADZONE) left_strength = NK_MAX(left_strength, -right_x * 5.0f);
    if (right_x > NK_CONSOLE_AXIS_DEADZONE) right_strength = NK_MAX(right_strength, right_x * 5.0f);
    if (right_y < -NK_CONSOLE_AXIS_DEADZONE) up_strength = NK_MAX(up_strength, -right_y * 5.0f);
    if (right_y > NK_CONSOLE_AXIS_DEADZONE) down_strength = NK_MAX(down_strength, right_y * 5.0f);
    if (ltrig > NK_CONSOLE_AXIS_DEADZONE) up_strength = NK_MAX(up_strength, ltrig * 10.0f);
    if (rtrig > NK_CONSOLE_AXIS_DEADZONE) down_strength = NK_MAX(down_strength, rtrig * 10.0f);

    // Up/Down
    if (nk_console_axis_tick(&data->axis_ud, NK_MAX(up_strength, down_strength), console->ctx->delta_time_seconds)) {
        if (up_strength >= down_strength) {
            data->axis_up_fired = nk_true;
        }
        else {
            data->axis_down_fired = nk_true;
        }
    }
    // Left/Right
    if (nk_console_axis_tick(&data->axis_lr, NK_MAX(left_strength, right_strength), console->ctx->delta_time_seconds)) {
        if (left_strength >= right_strength)
            data->axis_left_fired = nk_true;
        else
            data->axis_right_fired = nk_true;
    }
}

/**
 * Handles the Touch and Drag Scrolling.
 */
static void nk_console_window_touch_drag(nk_console* console, nk_console_top_data* top_data) {
    struct nk_input* in = &console->ctx->input;
    if (nk_window_is_hovered(console->ctx) && nk_input_is_mouse_pressed(in, NK_BUTTON_LEFT)) {
        struct nk_rect content = nk_window_get_content_region(console->ctx);
        top_data->drag_scroll_initiated = nk_input_is_mouse_hovering_rect(in, content);
        if (top_data->drag_scroll_initiated) {
            top_data->drag_scroll_origin = in->mouse.pos;
            nk_window_get_scroll(console->ctx, &top_data->drag_scroll_start_x, &top_data->drag_scroll_start_y);
            top_data->drag_scroll_active = nk_false;
        }
    }
    if (nk_input_is_mouse_down(in, NK_BUTTON_LEFT)) {
        if (top_data->drag_scroll_initiated) {
            float dy = top_data->drag_scroll_origin.y - in->mouse.pos.y;
            float dx = top_data->drag_scroll_origin.x - in->mouse.pos.x;
            if (!top_data->drag_scroll_active &&
                (dy * dy + dx * dx) > NK_CONSOLE_DRAG_THRESHOLD * NK_CONSOLE_DRAG_THRESHOLD) {
                top_data->drag_scroll_active = nk_true;
            }
            if (top_data->drag_scroll_active) {
                nk_uint sx = (nk_uint)NK_CLAMP(0.0f, (float)top_data->drag_scroll_start_x + dx, (float)top_data->drag_scroll_max_x);
                nk_uint sy = (nk_uint)NK_CLAMP(0.0f, (float)top_data->drag_scroll_start_y + dy, (float)top_data->drag_scroll_max_y);
                nk_window_set_scroll(console->ctx, sx, sy);
                top_data->input_processed = nk_true;
            }
        }
    }
    else {
        top_data->drag_scroll_active = nk_false;
        top_data->drag_scroll_initiated = nk_false;
    }
}

/**
 * Ensures a selectable active widget exists in parent, picking the first selectable if needed.
 *
 * @param parent The parent console widget whose active widget should be validated.
 */
static void nk_console_ensure_active_widget(nk_console* parent) {
    if (parent->activeWidget == NULL) {
        nk_console_set_active_widget(nk_console_find_first_selectable(parent));
        return;
    }
    for (size_t i = 0; i < cvector_size(parent->children); ++i) {
        if (parent->children[i] == parent->activeWidget) {
            if (nk_console_selectable(parent->activeWidget)) {
                return;
            }
            break;
        }
    }
    nk_console_set_active_widget(nk_console_find_first_selectable(parent));
}

/**
 * Invokes and erases all one-shot post-render events on the given console.
 *
 * @param console The top-level console whose post-render event queue should be processed.
 */
static void nk_console_process_post_render_events(nk_console* console) {
    size_t count = cvector_size(console->events);
    if (count == 0) {
        return;
    }
    for (size_t i = count; i-- > 0; ) {
        if (console->events[i].type == NK_CONSOLE_EVENT_POST_RENDER_ONCE) {
            if (console->events[i].callback != NULL) {
                console->events[i].callback((nk_console*)console->events[i].user_data, NULL);
            }
            cvector_erase(console->events, i);
        }
    }
}

/**
 * Recomputes drag-scroll bounds from the current window layout.
 *
 * @param console The top-level console widget.
 * @param data The top-level data associated with the console.
 */
static void nk_console_update_drag_scroll(nk_console* console, nk_console_top_data* data) {
    if (console->ctx->current == NULL) {
        return;
    }
    struct nk_rect content = nk_window_get_content_region(console->ctx);
    float rendered_h = console->ctx->current->layout->at_y - console->ctx->current->layout->bounds.y + console->ctx->current->layout->row.height;
    data->drag_scroll_max_y = (nk_uint)NK_MAX(0.0f, rendered_h - content.h);
    data->drag_scroll_max_x = (nk_uint)NK_MAX(0.0f, console->ctx->current->layout->max_x - console->ctx->current->layout->bounds.x - content.w);
}

/**
 * Renders the top-level console: processes input, renders children, and handles post-render events.
 *
 * @param console The top-level console widget to render.
 */
static void nk_console_render_top(nk_console* console) {
    nk_console_top_data* data = (nk_console_top_data*)console->data;
    data->input_processed = nk_false;
    nk_console_axis_update(console);
    nk_console_window_touch_drag(console, data);
    nk_console_trigger_event(data->active_parent, NK_CONSOLE_EVENT_PRE_PARENT_RENDER);
    nk_console_render_message(console);

    if (data->active_parent->children != NULL) {
        nk_console_ensure_active_widget(data->active_parent);
        for (size_t i = 0; i < cvector_size(data->active_parent->children); ++i) {
            nk_console_render(data->active_parent->children[i]);
        }
    }

    nk_console_process_post_render_events(console);
    // Update drag scroll bounds so they're valid when called directly (e.g. SDL/GLFW) rather than via nk_console_render_window.
    nk_console_update_drag_scroll(console, data);
}

NK_API void nk_console_render(nk_console* console) {
    if (console == NULL || console->visible == nk_false) {
        return;
    }

    if (console->parent == NULL) {
        nk_console_render_top(console);
        return;
    }

    struct nk_rect widget_bounds = console->render != NULL ? console->render(console) : nk_rect(0, 0, 0, 0);
    if (widget_bounds.w <= 0 || widget_bounds.h <= 0) {
        return;
    }

    nk_console* top = nk_console_get_top(console);
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;

    // When nk_console_navigate_back targeted this widget, scroll the window to show it.
    if (top_data->scroll_to_widget == console) {
        struct nk_rect content_region = nk_window_get_content_region(console->ctx);
        nk_uint offsetx, offsety;
        nk_window_get_scroll(console->ctx, &offsetx, &offsety);
        if (widget_bounds.y + widget_bounds.h > content_region.y + content_region.h + (float)offsety) {
            nk_window_set_scroll(console->ctx, offsetx, (nk_uint)(widget_bounds.y + widget_bounds.h - content_region.y - content_region.h));
        }
        else if (widget_bounds.y < content_region.y + (float)offsety) {
            nk_window_set_scroll(console->ctx, offsetx, (nk_uint)(widget_bounds.y - content_region.y));
        }
        top_data->scroll_to_widget = NULL;
    }

    // Allow the mouse to switch focus to the widget.
    if (nk_input_is_mouse_moved(&console->ctx->input)) {
        nk_uint window_scroll_x, window_scroll_y;
        nk_window_get_scroll(console->ctx, &window_scroll_x, &window_scroll_y);
        widget_bounds.x -= (float)window_scroll_x;
        widget_bounds.y -= (float)window_scroll_y;
        if (top_data->input_processed == nk_false && nk_input_is_mouse_hovering_rect(&console->ctx->input, widget_bounds)) {
            if (nk_console_selectable(console)) {
                nk_console_set_active_widget(console);
                top_data->input_processed = nk_true;
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
    data->axis_ud.timer = -1.0f;
    data->axis_lr.timer = -1.0f;
    data->gamepad_num = -1;
    data->active_parent = console;
    console->data = data;

    return console;
}

/**
 * Renders the given console to a new window with the given properties.
 *
 * By using this method, nuklear_console has the ability to determine whether or not the scrollbar should be active.
 *
 * @param console The console to render.
 * @param title The title of the window.
 * @param bounds The bounds of the window.
 * @param flags The flags to apply to the window.
 *
 * @return The resulting size of the window.
 */
NK_API struct nk_rect nk_console_render_window(nk_console* console, const char* title, struct nk_rect bounds, nk_uint flags) {
    nk_console_top_data* top_data;
    struct nk_rect window_bounds;
    if (console == NULL) {
        window_bounds.x = 0;
        window_bounds.y = 0;
        window_bounds.w = 0;
        window_bounds.h = 0;
        return window_bounds;
    }

    // Determine if the scrollbar is needed.
    top_data = (nk_console_top_data*)console->data;
    if ((flags & NK_WINDOW_SCROLL_AUTO_HIDE) != 0) {
        if (top_data->scrollbar_required) {
            flags |= (nk_uint)NK_WINDOW_NO_SCROLLBAR;
        }
        else {
            flags &= ~(nk_uint)NK_WINDOW_NO_SCROLLBAR;
        }
    }

    // Process the Nuklear window.
    if (nk_begin(console->ctx, title, bounds, flags)) {
        nk_console_render(console);
    }

    // After all elements have been rendered, update the layout flags.
    if ((flags & NK_WINDOW_SCROLL_AUTO_HIDE) != 0) {
        top_data->scrollbar_required = (console->ctx->current->layout->at_y - console->ctx->current->layout->bounds.y) <= console->ctx->current->layout->bounds.h;
        if (top_data->scrollbar_required) {
            console->ctx->current->layout->flags |= (nk_uint)NK_WINDOW_NO_SCROLLBAR;
        }
        else {
            console->ctx->current->layout->flags &= ~(nk_uint)NK_WINDOW_NO_SCROLLBAR;
        }
    }

    // Calculate the content size and scroll bounds.
    {
        struct nk_rect content = nk_window_get_content_region(console->ctx);
        float rendered_h = console->ctx->current->layout->at_y - console->ctx->current->layout->bounds.y + console->ctx->current->layout->row.height;
        window_bounds = nk_window_get_bounds(console->ctx);
        window_bounds.h = window_bounds.h - content.h + rendered_h;
    }
    nk_console_update_drag_scroll(console, top_data);

    // Finish the window processing.
    nk_end(console->ctx);
    return window_bounds;
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
    for (struct nk_console** it = cvector_begin(console->children); it != cvector_end(console->children); ++it) {
        if (*it != NULL) {
            nk_console_free(*it);
            *it = NULL;
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

NK_API void nk_console_set_gamepad_num(nk_console* console, int num) {
    nk_console* top = nk_console_get_top(console);
    if (top == NULL) {
        return;
    }
    nk_console_top_data* data = (nk_console_top_data*)top->data;
    if (data == NULL) {
        return;
    }
    data->gamepad_num = num;
}

NK_API int nk_console_get_gamepad_num(nk_console* console) {
    nk_console* top = nk_console_get_top(console);
    if (top == NULL) {
        return -1;
    }
    nk_console_top_data* data = (nk_console_top_data*)top->data;
    if (data == NULL) {
        return -1;
    }
    return data->gamepad_num;
}

NK_API void nk_console_navigate_back(nk_console* leaving_parent) {
    if (leaving_parent == NULL) {
        return;
    }
    nk_console* top = nk_console_get_top(leaving_parent);
    nk_console_top_data* data = (nk_console_top_data*)top->data;
    if (leaving_parent == top) {
        // Still trigger the back event if we're trying to leave the top.
        nk_console_trigger_event(leaving_parent, NK_CONSOLE_EVENT_BACK);
        data->input_processed = nk_true;
        return;
    }

    // Traverse up until we're not on a row or tree. Rows and trees can't act as
    // active parents (their children render through the container, not directly),
    // so keep skipping them and let target track the direct child of the
    // destination, so focus lands on a navigable widget.
    nk_console* target = leaving_parent;
    while (target != top && (target->type == NK_CONSOLE_ROW || target->type == NK_CONSOLE_TREE)) {
        target = (target->parent != NULL) ? target->parent : top;
    }

    // Verify the destination, and find the target.
    nk_console* destination = (target->parent != NULL) ? target->parent : top;
    while (destination != top && (destination->type == NK_CONSOLE_ROW || destination->type == NK_CONSOLE_TREE)) {
        target = destination;
        destination = (destination->parent != NULL) ? destination->parent : top;
    }

    nk_console_set_active_parent(destination);
    nk_console_set_active_widget(target);
    if (data != NULL) {
        data->scroll_to_widget = target;
        data->input_processed = nk_true;
    }
    nk_console_trigger_event(leaving_parent, NK_CONSOLE_EVENT_BACK);
}

NK_API nk_console* nk_console_find_by_path(nk_console* console, const char* path) {
    if (console == NULL || path == NULL) {
        return NULL;
    }

    nk_console* current = nk_console_get_top(console);
    if (path[0] == '\0') {
        return current;
    }
    if (path[0] == '/') {
        path++;
    }

    char segment[256];
    while (path[0] != '\0') {
        const char* slash = strchr(path, '/');
        int len = (slash != NULL) ? (int)(slash - path) : (int)nk_strlen(path);
        if (len == 0) {
            path++;
            continue;
        }

        int copy_len = len < 255 ? len : 255;
        NK_MEMCPY(segment, path, (nk_size)copy_len);
        segment[copy_len] = '\0';

        nk_console* found = NULL;
        if (current->children != NULL) {
            for (size_t i = 0; i < cvector_size(current->children); i++) {
                nk_console* child = current->children[i];
                if (child == NULL || child->label == NULL) {
                    continue;
                }
                int label_len = child->label_length > 0
                                    ? child->label_length
                                    : (int)nk_strlen(child->label);
                if (label_len == copy_len &&
                    strncmp(child->label, segment, (size_t)copy_len) == 0) {
                    found = child;
                    break;
                }
            }
        }
        if (found == NULL) {
            return NULL;
        }
        current = found;
        path += len;
        if (path[0] == '/') {
            path++;
        }
    }
    return current;
}

NK_API nk_bool nk_console_navigate_to_path(nk_console* console, const char* path) {
    nk_console* target = nk_console_find_by_path(console, path);
    if (target == NULL) {
        return nk_false;
    }

    if (target->children != NULL) {
        nk_console_set_active_parent(target);
    }
    else {
        if (target->parent != NULL) {
            nk_console_set_active_parent(target->parent);
        }
        nk_console_set_active_widget(target);
    }
    return nk_true;
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
    if (nk_gamepad_is_button_released(data->gamepads, data->gamepad_num, (enum nk_gamepad_button)button)) {
        return nk_true;
    }

    // Keyboard/Mouse/Axis
    switch (button) {
        case NK_GAMEPAD_BUTTON_UP: return data->axis_up_fired || nk_input_is_key_released(&console->ctx->input, NK_KEY_UP);
        case NK_GAMEPAD_BUTTON_DOWN: return data->axis_down_fired || nk_input_is_key_released(&console->ctx->input, NK_KEY_DOWN);
        case NK_GAMEPAD_BUTTON_LEFT: return data->axis_left_fired || nk_input_is_key_released(&console->ctx->input, NK_KEY_LEFT);
        case NK_GAMEPAD_BUTTON_RIGHT: return data->axis_right_fired || nk_input_is_key_released(&console->ctx->input, NK_KEY_RIGHT);
        case NK_GAMEPAD_BUTTON_A: return nk_input_is_key_released(&console->ctx->input, NK_KEY_ENTER);
        case NK_GAMEPAD_BUTTON_B:
            // Escape Key
            return nk_input_is_key_released(&console->ctx->input, NK_CONSOLE_KEY_BACK) ||
                   // Mouse Back Button
                   (nk_input_is_mouse_pressed(&console->ctx->input, NK_BUTTON_X1) && nk_window_is_hovered(console->ctx));
        // case NK_GAMEPAD_BUTTON_X: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_A);
        // case NK_GAMEPAD_BUTTON_Y: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_S);
        case NK_GAMEPAD_BUTTON_LB: return nk_input_is_key_released(&console->ctx->input, NK_KEY_UP) && nk_input_is_key_down(&console->ctx->input, NK_KEY_SHIFT);
        case NK_GAMEPAD_BUTTON_RB: return nk_input_is_key_released(&console->ctx->input, NK_KEY_DOWN) && nk_input_is_key_down(&console->ctx->input, NK_KEY_SHIFT);
        case NK_GAMEPAD_BUTTON_BACK:
            return nk_input_is_key_released(&console->ctx->input, NK_KEY_SHIFT);
            // case NK_GAMEPAD_BUTTON_START: return nk_input_is_key_pressed(&console->ctx->input, NK_KEY_UP);
    }

    return nk_false;
}

NK_API nk_bool nk_console_button_down(nk_console* console, int button) {
    if (console == NULL) {
        return nk_false;
    }

    if (console->parent != NULL) {
        console = nk_console_get_top(console);
    }

    // Gamepad
    nk_console_top_data* data = (nk_console_top_data*)console->data;
    if (nk_gamepad_is_button_down(data->gamepads, data->gamepad_num, (enum nk_gamepad_button)button)) {
        return nk_true;
    }

    // Keyboard/Mouse
    switch (button) {
        case NK_GAMEPAD_BUTTON_UP: return nk_input_is_key_down(&console->ctx->input, NK_KEY_UP);
        case NK_GAMEPAD_BUTTON_DOWN: return nk_input_is_key_down(&console->ctx->input, NK_KEY_DOWN);
        case NK_GAMEPAD_BUTTON_LEFT: return nk_input_is_key_down(&console->ctx->input, NK_KEY_LEFT);
        case NK_GAMEPAD_BUTTON_RIGHT: return nk_input_is_key_down(&console->ctx->input, NK_KEY_RIGHT);
    }

    return nk_false;
}

NK_API void nk_console_add_child(nk_console* parent, nk_console* child) {
    if (parent == NULL || child == NULL) {
        return;
    }

    // If we are adding it to a TREE, insert child as a sibling so navigation works naturally.
    if (parent->type == NK_CONSOLE_TREE && parent->parent != NULL && parent->data != NULL) {
        nk_console_tree_data* tree_data = (nk_console_tree_data*)parent->data;

        // Set the visibility of the child based on whether the tree is expanded.
        child->visible = nk_console_tree_expanded(parent);

        // Insert position: immediately after the tree and any previously owned children.
        int widget_index = nk_console_get_widget_index(parent);
        if (widget_index < 0) {
            child->parent = parent;
            cvector_push_back(parent->children, child);
            cvector_push_back(tree_data->referenced_children, child);
            return;
        }

        // We are adding it to the middle of the parent, so insert accordingly.
        size_t insert_pos = (size_t)widget_index + (size_t)1 + cvector_size(tree_data->referenced_children);
        insert_pos = NK_MIN(insert_pos, cvector_size(parent->parent->children));
        child->parent = parent->parent;
        cvector_insert(parent->parent->children, insert_pos, child);
        cvector_push_back(tree_data->referenced_children, child);
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
