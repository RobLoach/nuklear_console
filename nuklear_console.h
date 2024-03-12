#ifndef NK_CONSOLE_H__
#define NK_CONSOLE_H__

typedef enum {
    NK_CONSOLE_UNKNOWN,
    NK_CONSOLE_PARENT,
    NK_CONSOLE_LABEL,
    NK_CONSOLE_BUTTON
} nk_console_widget_type;

typedef struct nk_console {
    nk_console_widget_type type;
    const char* text;
    int alignment;

    struct nk_console* parent;
    struct nk_context* context;

    struct nk_console** children;

    struct nk_console* activeParent;

    void (*onclick)(struct nk_console*);

    struct nk_console* activeWidget;

    nk_bool selectable;

    nk_bool cursorJustMoved;

    void* user_data;
} nk_console;

NK_API void* nk_console_malloc(nk_handle unused, void *old, nk_size size);
NK_API void nk_console_mfree(nk_handle unused, void *ptr);
NK_API void nk_console_free(nk_console* console);
NK_API void nk_console_render(nk_console* console);
NK_API nk_console* nk_console_add_label(nk_console* parent, const char* text);
NK_API nk_console* nk_console_add_button_onclick(nk_console* parent, const char* text, void (*onclick)(nk_console*));
NK_API nk_console* nk_console_add_button(nk_console* parent, const char* text);
NK_API nk_console* nk_console_init(struct nk_context* context);

#endif

#ifdef NK_CONSOLE_IMPLEMENTATION
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
nk_console* nk_console_get_parent_top(nk_console* widget) {
    if (widget == NULL) {
        return NULL;
    }

    nk_console* parent = widget;
    while (parent->parent != NULL) {
        parent = parent->parent;
    }

    return parent;
}

NK_API void nk_console_move_cursor(nk_console* console, int diff) {
    if (console == NULL) {
        return;
    }

    nk_console* top = nk_console_get_parent_top(console);
    nk_console* parent = top->activeParent == NULL ? top : top->activeParent;
    if (parent->children == NULL) {
        return;
    }

    if (top->activeWidget == NULL) {
        top->activeWidget = parent->children == NULL ? NULL : parent->children[0];
    }

    size_t i;
    int widgetIndex = -1;
    for (i = 0; i < cvector_size(parent->children); ++i) {
        if (parent->children[i] == top->activeWidget) {
            widgetIndex = i;
            break;
        }
    }
    if (widgetIndex == -1) {
        top->activeWidget = parent->children[0];
    }

    if (diff > 0) {
        for (int i = widgetIndex + 1; i < cvector_size(parent->children); i++) {
            printf("Item Next: %s\n", parent->children[i]->text);
            if (parent->children[i]->selectable == nk_true) {
                diff--;
                top->activeWidget = parent->children[i];
                if (diff <= 0) {
                    printf("Item Next2222: %s\n", top->activeWidget->text);
                    //top->cursorJustMoved = nk_true;
                    break;
                }
            }
        }
    }
    else if (diff < 0) {
        for (int i = widgetIndex - 1; i >= 0; i--) {
            if (parent->children[i]->selectable == nk_true) {
                diff++;
                top->activeWidget = parent->children[i];
                if (diff >= 0) {
                    printf("Done move: %s\n", top->activeWidget->text);
                    //top->cursorJustMoved = nk_true;
                    break;
                }
            }
        }
    }
}

NK_API nk_bool nk_console_active_press(nk_console* console) {
    if (console == NULL) {
        return nk_false;
    }

    struct nk_input* input = &console->context->input;
    return nk_input_is_key_pressed(input, NK_KEY_ENTER);
}

NK_API void nk_console_allow_up_down(nk_console* console) {
    struct nk_input* input = &console->context->input;

    nk_console* top = nk_console_get_parent_top(console);
    if (top->cursorJustMoved == nk_false) {
        if (nk_input_is_key_pressed(input, NK_KEY_UP)) {
            nk_console_move_cursor(console, -1);
                    top->cursorJustMoved = nk_true;
        }
        else if (nk_input_is_key_pressed(input, NK_KEY_DOWN)) {
            nk_console_move_cursor(console, 1);
                    top->cursorJustMoved = nk_true;
        }
    }
}

NK_API void nk_console_render(nk_console* console) {
    if (console == NULL) {
        return;
    }

    // Render the active parent if required.
    if (console->activeParent != NULL && console->activeParent->children != NULL) {
        size_t i;
        for (i = 0; i < cvector_size(console->activeParent->children); ++i) {
            nk_console_render(console->activeParent->children[i]);
        }
        return;
    }

    nk_console* top = nk_console_get_parent_top(console);
    if (console->parent == NULL) {
        top->cursorJustMoved == nk_false;
        printf("SDATARDSF\n");
    }

    if (top->activeWidget == NULL) {
        nk_console_move_cursor(console, 0);
    }

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
            nk_label(console->context, console->text, console->alignment);
        }
        break;
        case NK_CONSOLE_BUTTON: {
            nk_layout_row_dynamic(console->context, 0, 1);
            //printf("Selected item: %s\n", top->activeWidget->text);

            nk_bool selected = top->activeWidget == console && nk_console_active_press(console);


            if (top->activeWidget == console) {
                struct nk_style_item swap = console->context->style.button.hover;
                console->context->style.button.hover = console->context->style.button.normal;
                console->context->style.button.normal = swap;
            }

            selected |= nk_button_label(console->context, console->text);

            if (top->activeWidget == console) {
                struct nk_style_item swap = console->context->style.button.hover;
                console->context->style.button.hover = console->context->style.button.normal;
                console->context->style.button.normal = swap;
            }

            if (selected) {
                if (console->onclick == NULL) {
                    if (console->children != NULL) {
                        nk_console* top = nk_console_get_parent_top(console);
                        if (top != NULL) {
                            top->activeParent = console;
                            nk_console_move_cursor(console, 0);
                        }
                    }
                }
                else {
                    console->onclick(console);
                }
            }
            else {
                if (top->activeWidget == console) {
                    nk_console_allow_up_down(console);
                }
            }
        }
        break;
    }
}

NK_API void* nk_console_malloc(nk_handle unused, void *old, nk_size size) {
    return NK_CONSOLE_MALLOC(unused, old, size);
}

NK_API void nk_console_mfree(nk_handle unused, void *ptr) {
    NK_CONSOLE_FREE(unused, ptr);
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

NK_API nk_console* nk_console_add_button(nk_console* parent, const char* text) {
    return nk_console_add_button_onclick(parent, text, NULL);
}

NK_API void nk_console_onclick_back(nk_console* button) {
    nk_console* top = nk_console_get_parent_top(button);
    if (top != NULL) {
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
}

NK_API nk_console* nk_console_add_button_onclick(nk_console* parent, const char* text, void (*onclick)(struct nk_console*)) {
    nk_console* button = nk_console_add_label(parent, text);
    button->type = NK_CONSOLE_BUTTON;
    button->onclick = onclick;
    button->selectable = nk_true;
    return button;
}

NK_API nk_console* nk_console_init(struct nk_context* context) {
    nk_handle handle;
    nk_console* console = nk_console_malloc(handle, NULL, sizeof(nk_console));
    console->type = NK_CONSOLE_PARENT;
    console->text = "";
    console->activeWidget = NULL;
    console->alignment = 0;
    console->parent = NULL;
    console->context = context;
    console->children = NULL;
    console->activeParent = NULL;
    console->onclick = NULL;
    console->selectable = nk_false;
    console->user_data = NULL;
    console->cursorJustMoved = nk_false;
    return console;
}

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