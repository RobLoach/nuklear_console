#ifndef NK_CONSOLE_H__
#define NK_CONSOLE_H__

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

struct nk_console;

typedef struct nk_console_combobox_data {
    const char* label;
    const char* items_separated_by_separator;
    int separator;
    int* selected;
    int count;
} nk_console_combobox_data;
struct nk_console;

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

typedef struct nk_console_button_data {
    enum nk_symbol_type symbol;
    void (*onclick)(struct nk_console*);
    nk_bool skip_row;
    int text_length;
} nk_console_button_data;

typedef struct nk_console {
    nk_console_widget_type type;
    const char* text;
    int alignment;

    nk_bool selectable;
    nk_bool* value_bool;
    nk_size* value_size;
    nk_size max_size;

    nk_console_combobox_data combobox;
    nk_console_button_data button;
    nk_console_property_data property;

    struct nk_console* parent;
    struct nk_context* context;
    struct nk_console** children;
    struct nk_console* activeParent;
    struct nk_console* activeWidget;
    nk_bool input_processed;

    void (*onchange)(struct nk_console*);

    const char* description;
} nk_console;

NK_API nk_console* nk_console_init(struct nk_context* context);
NK_API void nk_console_free(nk_console* console);
NK_API void nk_console_render(nk_console* console);
NK_API nk_console* nk_console_add_button_onclick(nk_console* parent, const char* text, void (*onclick)(nk_console*));
NK_API nk_console* nk_console_add_button(nk_console* parent, const char* text);
NK_API nk_console* nk_console_add_checkbox(nk_console* parent, const char* text, nk_bool* active);
NK_API nk_console* nk_console_add_combobox(nk_console* parent, const char* label, const char *items_separated_by_separator, int separator, int* selected);
NK_API nk_console* nk_console_add_progress(nk_console* parent, const char* text, nk_size* current, nk_size max);
NK_API nk_console* nk_console_add_property_int(nk_console* parent, const char* label, int min, int *val, int max, int step, float inc_per_pixel);
NK_API nk_console* nk_console_add_property_float(nk_console* parent, const char* label, float min, float *val, float max, float step, float inc_per_pixel);
NK_API nk_console* nk_console_add_label(nk_console* parent, const char* text);
NK_API nk_console* nk_console_add_slider_int(nk_console* parent, const char* label, int min, int* val, int max, int step);
NK_API nk_console* nk_console_add_slider_float(nk_console* parent, const char* label, float min, float* val, float max, float step);
NK_API void nk_console_onclick_back(nk_console* button);
NK_API nk_console* nk_console_get_top(nk_console* widget);
NK_API void* nk_console_malloc(nk_handle unused, void *old, nk_size size);
NK_API void nk_console_mfree(nk_handle unused, void *ptr);

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
#include "vendor/c-vector/cvector.h"

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
int nk_console_get_widget_index(nk_console* widget) {
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
void nk_console_check_up_down(nk_console* widget, struct nk_rect bounds) {
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
                if (target != NULL && target->selectable) {
                    top->activeWidget = target;
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
                if (target != NULL && target->selectable) {
                    top->activeWidget = target;
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
                    top->activeWidget = NULL;
                }
                else if (widget->parent->parent != NULL) {
                    top->activeParent = widget->parent->parent;
                    top->activeWidget = NULL;
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
            if (parent->children[i]->selectable) {
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

NK_API void nk_console_tooltip(nk_console* console, struct nk_rect widget_bounds) {
    if (console == NULL) {
        return;
    }

    if (console->description != NULL) {
        int x = console->context->input.mouse.pos.x;
        int y = console->context->input.mouse.pos.y;
        console->context->input.mouse.pos.x = widget_bounds.x;
        console->context->input.mouse.pos.y = widget_bounds.y + widget_bounds.h;
        nk_tooltip(console->context, console->description);
        console->context->input.mouse.pos.x = x;
        console->context->input.mouse.pos.y = y;
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
            console->activeWidget = nk_console_find_first_selectable(console->activeParent != NULL ? console->activeParent : console);
        }

        // Render the active parent.
        if (console->activeParent != NULL && console->activeParent->children != NULL) {
            size_t i;
            for (i = 0; i < cvector_size(console->activeParent->children); ++i) {
                nk_console_render(console->activeParent->children[i]);
            }
            return;
        }
    }

    nk_console* top = nk_console_get_top(console);
    struct nk_rect widget_bounds = {0,0,0,0};

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
        case NK_CONSOLE_LABEL: {
            nk_layout_row_dynamic(console->context, 0, 1);

            // TODO: Add label options like alignment or text wrapping
            nk_label(console->context, console->text, console->alignment);
        }
        break;
        case NK_CONSOLE_BUTTON: {
            if (!console->button.skip_row) {
                nk_layout_row_dynamic(console->context, 0, 1);
            }
            widget_bounds = nk_layout_widget_bounds(console->context);

            // Check the button state.
            nk_bool selected = nk_false;
            if (top->activeWidget == console && !top->input_processed && nk_input_is_key_pressed(&console->context->input, NK_KEY_ENTER)) {
                selected = nk_true;
            }

            // Apply the style.
            struct nk_style_item buttonStyle = console->context->style.button.normal;
            if (top->activeWidget == console) {
                if (selected) {
                    console->context->style.button.normal = console->context->style.button.active;
                }
                else {
                    console->context->style.button.normal = console->context->style.button.hover;
                }
            }

            // Display the button.
            if (console->button.text_length <= 0) {
                if (console->button.symbol == NK_SYMBOL_NONE) {
                    selected |= nk_button_label(console->context, console->text);
                }
                else {
                    selected |= nk_button_symbol_label(console->context, console->button.symbol, console->text, console->alignment);
                }
            }
            else {
                if (console->button.symbol == NK_SYMBOL_NONE) {
                    selected |= nk_button_text(console->context, console->text, console->button.text_length);
                }
                else {
                    selected |= nk_button_symbol_text(console->context, console->button.symbol, console->text, console->button.text_length, console->alignment);
                }
            }

            // Restore the styles
            console->context->style.button.normal = buttonStyle;

            // Act on the button
            if (selected) {
                top->input_processed = nk_true;

                // If there's no onclick action and there are children...
                if (console->button.onclick == NULL) {
                    if (console->children != NULL) {
                        top->activeParent = console;
                        top->activeWidget = NULL;
                    }
                }
                else {
                    console->button.onclick(console);
                }
            }

            // Allow switching up/down in widgets
            if (top->activeWidget == console) {
                nk_console_check_up_down(console, widget_bounds);
                nk_console_tooltip(console, widget_bounds);
            }
        }
        break;
        case NK_CONSOLE_CHECKBOX: {
            nk_layout_row_dynamic(console->context, 0, 1);
            widget_bounds = nk_layout_widget_bounds(console->context);

            // Allow changing the checkbox value.
            nk_bool active = nk_false;
            if (top->activeWidget == console && !top->input_processed) {
                if (nk_input_is_key_pressed(&console->context->input, NK_KEY_ENTER)) {
                    if (console->value_bool != NULL) {
                        *console->value_bool = !*console->value_bool;
                        if (console->onchange != NULL) {
                            console->onchange(console);
                        }
                    }
                    active = nk_true;
                    top->input_processed = nk_true;
                }
                else if (nk_input_is_key_pressed(&console->context->input, NK_KEY_LEFT)) {
                    if (console->value_bool != NULL) {
                        *console->value_bool = nk_false;
                        if (console->onchange != NULL) {
                            console->onchange(console);
                        }
                    }
                    active = nk_true;
                    top->input_processed = nk_true;
                }
                else if (nk_input_is_key_pressed(&console->context->input, NK_KEY_RIGHT)) {
                    if (console->value_bool != NULL) {
                        *console->value_bool = nk_true;
                        if (console->onchange != NULL) {
                            console->onchange(console);
                        }
                    }
                    active = nk_true;
                    top->input_processed = nk_true;
                }
            }

            // Style
            struct nk_style_item checkboxStyle = console->context->style.checkbox.normal;
            if (top->activeWidget == console) {
                if (active) {
                    console->context->style.checkbox.normal = console->context->style.checkbox.active;
                }
                else {
                    console->context->style.checkbox.normal = console->context->style.checkbox.hover;
                }
            }

            // Display the checkbox with fixed alignment.
            if (console->alignment == NK_TEXT_LEFT) {
                nk_checkbox_label_align(console->context, console->text, console->value_bool, NK_TEXT_RIGHT, NK_TEXT_LEFT);
            }
            else {
                nk_checkbox_label(console->context, console->text, console->value_bool);
            }

            // Restore the styles
            console->context->style.checkbox.normal = checkboxStyle;

            // Allow switching up/down in widgets
            if (top->activeWidget == console) {
                nk_console_check_up_down(console, widget_bounds);
                nk_console_tooltip(console, widget_bounds);
            }
        }
        break;
        case NK_CONSOLE_PROGRESS: {
            nk_layout_row_dynamic(console->context, 0, 2);

            // Allow changing the value.
            nk_bool active = nk_false;
            if (top->activeWidget == console && !top->input_processed) {
                if (nk_input_is_key_pressed(&console->context->input, NK_KEY_LEFT)) {
                    if (console->value_size != NULL && *console->value_size > 0) {
                        *console->value_size = *console->value_size - 1;
                        if (console->onchange != NULL) {
                            console->onchange(console);
                        }
                    }
                    active = nk_true;
                    top->input_processed = nk_true;
                }
                else if (nk_input_is_key_pressed(&console->context->input, NK_KEY_RIGHT)) {
                    if (console->value_size != NULL && *console->value_size < console->max_size) {
                        *console->value_size = *console->value_size + 1;
                        if (console->onchange != NULL) {
                            console->onchange(console);
                        }
                    }
                    active = nk_true;
                    top->input_processed = nk_true;
                }
            }

            // Display the label
            nk_label(console->context, console->text, NK_TEXT_LEFT);
            widget_bounds = nk_layout_widget_bounds(console->context);

            // Progress
            struct nk_style_item cursor_normal = console->context->style.progress.cursor_normal;
            struct nk_style_item cursor_hover = console->context->style.progress.cursor_hover;
            struct nk_style_item cursor_active = console->context->style.progress.cursor_active;
            if (top->activeWidget == console) {
                if (active) {
                    console->context->style.progress.cursor_normal = cursor_active;
                }
                else {
                    console->context->style.progress.cursor_normal = cursor_active;
                }
            }

            // Display the widget
            nk_progress(console->context, console->value_size, console->max_size, nk_true);

            // Restore the styles
            console->context->style.progress.cursor_normal = cursor_normal;
            console->context->style.progress.cursor_hover = cursor_hover;
            console->context->style.progress.cursor_active = cursor_active;

            // Allow switching up/down in widgets
            if (top->activeWidget == console) {
                nk_console_check_up_down(console, widget_bounds);
                nk_console_tooltip(console, widget_bounds);
            }
        }
        break;
        case NK_CONSOLE_COMBOBOX: {
            nk_layout_row_dynamic(console->context, 0, 2);

            // Allow changing the value with left/right
            if (top->activeWidget == console && !top->input_processed) {
                if (console->combobox.selected != NULL && console->children != NULL) {
                    nk_bool changed = nk_false;
                    if (nk_input_is_key_pressed(&console->context->input, NK_KEY_LEFT) && *console->combobox.selected > 0) {
                        *console->combobox.selected = *console->combobox.selected - 1;
                        changed = nk_true;
                    }
                    else if (nk_input_is_key_pressed(&console->context->input, NK_KEY_RIGHT) && *console->combobox.selected < cvector_size(console->children) - 2) {
                        *console->combobox.selected = *console->combobox.selected + 1;
                        changed = nk_true;
                    }

                    if (changed) {
                        console->text = console->children[*console->combobox.selected + 1]->text;
                        console->button.text_length = console->children[*console->combobox.selected + 1]->button.text_length;
                        if (console->onchange != NULL) {
                            console->onchange(console);
                        }
                    }
                }
            }

            // Display the label
            nk_label(console->context, console->combobox.label, NK_TEXT_LEFT);
            widget_bounds = nk_layout_widget_bounds(console->context);

            // Display the mocked combobox button
            console->type = NK_CONSOLE_BUTTON;
            if (top->activeWidget == console) {
                console->button.symbol = NK_SYMBOL_TRIANGLE_DOWN;
            }
            else {
                console->button.symbol = NK_SYMBOL_NONE;
            }
            nk_console_render(console);
            console->type = NK_CONSOLE_COMBOBOX;

            // Allow switching up/down in widgets
            if (top->activeWidget == console) {
                nk_console_check_up_down(console, widget_bounds);
                nk_console_tooltip(console, widget_bounds);
            }
        }
        break;
        case NK_CONSOLE_SLIDER_INT:
        case NK_CONSOLE_SLIDER_FLOAT:
        case NK_CONSOLE_PROPERTY_INT:
        case NK_CONSOLE_PROPERTY_FLOAT: {
            nk_layout_row_dynamic(console->context, 0, 2);

            // Allow changing the value with left/right
            if (top->activeWidget == console && !top->input_processed) {
                if (nk_input_is_key_pressed(&console->context->input, NK_KEY_LEFT)) {
                    switch (console->type) {
                        case NK_CONSOLE_SLIDER_INT:
                        case NK_CONSOLE_PROPERTY_INT:
                            *console->property.val_int = *console->property.val_int - console->property.step_int;
                            if (*console->property.val_int < console->property.min_int) {
                                *console->property.val_int = console->property.min_int;
                            }
                            break;
                        case NK_CONSOLE_SLIDER_FLOAT:
                        case NK_CONSOLE_PROPERTY_FLOAT:
                            *console->property.val_float = *console->property.val_float - console->property.step_float;
                            if (*console->property.val_float < console->property.min_float) {
                                *console->property.val_float = console->property.min_float;
                            }
                            break;
                    }
                    if (console->onchange != NULL) {
                        console->onchange(console);
                    }
                    top->input_processed = nk_true;
                }
                else if (nk_input_is_key_pressed(&console->context->input, NK_KEY_RIGHT)) {
                    switch (console->type) {
                        case NK_CONSOLE_SLIDER_INT:
                        case NK_CONSOLE_PROPERTY_INT:
                            *console->property.val_int = *console->property.val_int + console->property.step_int;
                            if (*console->property.val_int > console->property.max_int) {
                                *console->property.val_int = console->property.max_int;
                            }
                            break;
                        case NK_CONSOLE_SLIDER_FLOAT:
                        case NK_CONSOLE_PROPERTY_FLOAT:
                            *console->property.val_float = *console->property.val_float + console->property.step_float;
                            if (*console->property.val_float > console->property.max_float) {
                                *console->property.val_float = console->property.max_float;
                            }
                            break;
                    }
                    if (console->onchange != NULL) {
                        console->onchange(console);
                    }
                    top->input_processed = nk_true;
                }
            }

            // Style
            enum nk_symbol_type left = console->context->style.property.sym_left;
            enum nk_symbol_type right = console->context->style.property.sym_right;
            struct nk_color bar_normal = console->context->style.slider.bar_normal;
            struct nk_style_item cursor_normal = console->context->style.slider.cursor_normal;

            if (top->activeWidget != console) {
                console->context->style.property.sym_left = NK_SYMBOL_NONE;
                console->context->style.property.sym_right = NK_SYMBOL_NONE;
            }
            else {
                console->context->style.slider.bar_normal = console->context->style.slider.bar_hover;
                console->context->style.slider.cursor_normal = console->context->style.slider.cursor_hover;
            }

            // Display the label
            nk_label(console->context, console->text, NK_TEXT_LEFT);
            widget_bounds = nk_layout_widget_bounds(console->context);

            // Display the widget
            switch (console->type) {
                case NK_CONSOLE_PROPERTY_INT:
                    nk_property_int(console->context, "", console->property.min_int, console->property.val_int, console->property.max_int, console->property.step_int, console->property.inc_per_pixel);
                    break;
                case NK_CONSOLE_PROPERTY_FLOAT:
                   nk_property_float(console->context, "", console->property.min_float, console->property.val_float, console->property.max_float, console->property.step_float, console->property.inc_per_pixel);
                   break;
                case NK_CONSOLE_SLIDER_INT:
                    nk_slider_int(console->context, console->property.min_int, console->property.val_int, console->property.max_int, console->property.step_int);
                    break;
                case NK_CONSOLE_SLIDER_FLOAT:
                    nk_slider_float(console->context, console->property.min_float, console->property.val_float, console->property.max_float, console->property.step_float);
                    break;
            }

            // Style Restoration
            if (top->activeWidget != console) {
                console->context->style.property.sym_left = left;
                console->context->style.property.sym_right = right;
            }
            else {
                console->context->style.slider.bar_normal = bar_normal;
                console->context->style.slider.cursor_normal = cursor_normal;

            }

            // Allow switching up/down in widgets
            if (top->activeWidget == console && !top->input_processed) {
                nk_console_check_up_down(console, widget_bounds);
                nk_console_tooltip(console, widget_bounds);
            }
        }
        break;
    }

    // Allow mouse to switch focus between active widgets
    if (top->input_processed == nk_false && widget_bounds.w > 0 && nk_input_is_mouse_moved(&console->context->input) && nk_input_is_mouse_hovering_rect(&console->context->input, widget_bounds)) {
        top->activeWidget = console;
        top->input_processed = nk_true;
    }
}

NK_API void* nk_console_malloc(nk_handle unused, void *old, nk_size size) {
    return NK_CONSOLE_MALLOC(unused, old, size);
}

NK_API void nk_console_mfree(nk_handle unused, void *ptr) {
    NK_CONSOLE_FREE(unused, ptr);
}

NK_API nk_console* nk_console_add_checkbox(nk_console* parent, const char* text, nk_bool* active) {
    nk_console* checkbox = nk_console_add_label(parent, text);
    checkbox->value_bool = active;
    checkbox->type = NK_CONSOLE_CHECKBOX;
    checkbox->selectable = nk_true;
    return checkbox;
}

NK_API nk_console* nk_console_add_label(nk_console* parent, const char* text) {
    nk_console* label = nk_console_init(parent->context);
    label->type = NK_CONSOLE_LABEL;
    label->text = text;
    label->parent = parent;
    label->alignment = NK_TEXT_LEFT;
    cvector_push_back(parent->children, label);
    return label;
}

NK_API nk_console* nk_console_add_property_int(nk_console* parent, const char* label, int min, int *val, int max, int step, float inc_per_pixel) {
    NK_ASSERT(val);
    nk_console* widget = nk_console_add_label(parent, label);
    widget->type = NK_CONSOLE_PROPERTY_INT;
    widget->selectable = nk_true;
    widget->property.min_int = min;
    widget->property.val_int = val;
    widget->property.max_int = max;
    widget->property.step_int = step;
    widget->property.inc_per_pixel = inc_per_pixel;
    if (*val < min) {
        *val = min;
    }
    else if (*val > max) {
        *val = max;
    }
    return widget;
}

NK_API nk_console* nk_console_add_property_float(nk_console* parent, const char* label, float min, float *val, float max, float step, float inc_per_pixel) {
    NK_ASSERT(val);
    nk_console* widget = nk_console_add_label(parent, label);
    widget->type = NK_CONSOLE_PROPERTY_FLOAT;
    widget->selectable = nk_true;
    widget->property.min_float = min;
    widget->property.val_float = val;
    widget->property.max_float = max;
    widget->property.step_float = step;
    widget->property.inc_per_pixel = inc_per_pixel;
    if (*val < min) {
        *val = min;
    }
    else if (*val > max) {
        *val = max;
    }
    return widget;
}

NK_API nk_console* nk_console_add_slider_int(nk_console* parent, const char* label, int min, int* val, int max, int step) {
    nk_console* widget = nk_console_add_property_int(parent, label, min, val, max, step, 0);
    widget->type = NK_CONSOLE_SLIDER_INT;
    return widget;
}

NK_API nk_console* nk_console_add_slider_float(nk_console* parent, const char* label, float min, float* val, float max, float step) {
    nk_console* widget = nk_console_add_property_float(parent, label, min, val, max, step, 0);
    widget->type = NK_CONSOLE_SLIDER_FLOAT;
    return widget;
}

NK_API nk_console* nk_console_add_button(nk_console* parent, const char* text) {
    return nk_console_add_button_onclick(parent, text, NULL);
}

/**
 * Take action on a BACK button.
 */
NK_API void nk_console_onclick_back(nk_console* button) {
    if (button == NULL) {
        return;
    }

    nk_console* top = nk_console_get_top(button);
    if (top == NULL) {
        return;
    }

    nk_console* parent = button->parent;
    if (parent != NULL) {
        parent = parent->parent;
    }
    if (parent != NULL) {
        top->activeParent = parent;
        top->activeWidget = NULL;
    }
    else {
        top->activeParent = NULL;
        top->activeWidget = NULL;
    }
}

NK_API nk_console* nk_console_add_button_onclick(nk_console* parent, const char* text, void (*onclick)(struct nk_console*)) {
    nk_console* button = nk_console_add_label(parent, text);
    button->type = NK_CONSOLE_BUTTON;
    button->button.onclick = onclick;
    button->selectable = nk_true;
    return button;
}

NK_API nk_console* nk_console_add_progress(nk_console* parent, const char* text, nk_size* current, nk_size max) {
    nk_console* progress = nk_console_add_label(parent, text);
    progress->type = NK_CONSOLE_PROGRESS;
    progress->selectable = nk_true;
    progress->value_size = current;
    progress->max_size = max;
    if (*current < 0) {
        *current = 0;
    }
    else if (*current > max) {
        *current = max;
    }
    return progress;
}

/**
 * Handle the click event for combobox's children items.
 */
NK_API void nk_console_combobox_button_click(nk_console* button) {
    nk_console* combobox = button->parent;

    // Find which option was selected.
    int selected = nk_console_get_widget_index(button);
    if (selected <= 0 || selected >= cvector_size(combobox->children)) {
        nk_console_onclick_back(button);
        nk_console_get_top(combobox)->activeWidget = combobox;
        return;
    }

    // Update the active selected value.
    if (combobox->combobox.selected != NULL) {
        *combobox->combobox.selected = selected - 1;
    }

    // Change the combobox text that's displayed.
    combobox->text = button->text;
    combobox->button.text_length = button->button.text_length;

    // Go back
    nk_console_onclick_back(button);

    // Update the active widget to the combobox.
    nk_console_get_top(combobox)->activeWidget = combobox;

    // Invoke the onchange callback.
    if (combobox->onchange != NULL) {
        combobox->onchange(combobox);
    }
}

/**
 * Handle the click event for the main button for the combobox.
 *
 * @see nk_console_add_combobox
 * @internal
 */
NK_API void nk_console_combobox_button_main_click(nk_console* button) {
    nk_console* top = nk_console_get_top(button);
    int selected = button->combobox.selected == NULL ? 0 : *button->combobox.selected;
    if (button->children != NULL) {
        if (cvector_size(button->children) > selected + 1) {
            top->activeWidget = button->children[selected + 1];
        }
    }

    // Switch to show all the children.
    top->activeParent = button;
}

NK_API nk_console* nk_console_add_combobox(nk_console* parent, const char* label, const char *items_separated_by_separator, int separator, int* selected) {
    nk_console* combobox = nk_console_add_label(parent, label);
    combobox->type = NK_CONSOLE_COMBOBOX;
    combobox->selectable = nk_true;
    combobox->combobox.items_separated_by_separator = items_separated_by_separator;
    combobox->combobox.separator = separator;
    combobox->combobox.selected = selected;
    combobox->combobox.label = label;
    combobox->button.skip_row = nk_true;
    combobox->button.symbol = NK_SYMBOL_TRIANGLE_DOWN;

    // Back button
    nk_console_add_button_onclick(combobox, label, nk_console_combobox_button_click)
        ->button.symbol = NK_SYMBOL_TRIANGLE_UP;

    // Add all the sub-page buttons
    const char* button_text_start = items_separated_by_separator;
    int text_length = 0;
    for (int i = 0; items_separated_by_separator[i] != 0; i++) {
        text_length++;
        if (items_separated_by_separator[i] == (char)separator) {
            nk_console_add_button_onclick(combobox, button_text_start, nk_console_combobox_button_click)
                ->button.text_length = text_length - 1;
            text_length = 0;
            button_text_start = items_separated_by_separator + i + 1;
        }
    }

    // Add the last item
    nk_console_add_button_onclick(combobox, button_text_start, nk_console_combobox_button_click)
                ->button.text_length = text_length;

    if (selected != NULL) {
        if (*selected < 0) {
            *selected = 0;
        }
        else if (*selected >= cvector_size(combobox->children) - 1) {
            *selected = cvector_size(combobox->children) - 2;
        }

        combobox->text = combobox->children[*selected + 1]->text;
        combobox->button.text_length = combobox->children[*selected + 1]->button.text_length;
    }

    combobox->button.onclick = nk_console_combobox_button_main_click;

    return combobox;
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
