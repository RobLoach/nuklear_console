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
    nk_bool selecting;

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
NK_API void nk_console_update(nk_console* console);

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

NK_API void nk_console_update(nk_console* console) {
    if (console == NULL) {
        return;
    }

    struct nk_input* input = &console->context->input;

    if (console->selecting == 1) {
        console->selecting = 0;
    }

    if (nk_input_is_key_pressed(input, NK_KEY_ENTER)) {
        console->selecting = 1;
    }

    if (console->activeWidget == NULL) {
        nk_console* activeParent = console->activeParent == NULL ? console : console->activeParent;
        if (activeParent->children != NULL) {
            console->activeWidget = activeParent->children[0];
        }
    }

    if (console->activeWidget != NULL) {
        nk_console* parent = console->activeParent == NULL ? console : console->activeParent;
        if (parent->children != NULL) {
            size_t i;
            int widgetIndex = -1;
            for (i = 0; i < cvector_size(parent->children); ++i) {
                if (parent->children[i] == console->activeWidget) {
                    widgetIndex = i;
                    break;
                }
            }
            if (widgetIndex != -1) {
                if (nk_input_is_key_pressed(input, NK_KEY_DOWN)) {
                    widgetIndex++;
                    if (widgetIndex < cvector_size(parent->children)) {
                        console->activeWidget = parent->children[widgetIndex];
                        printf("Item: %s\n", console->activeWidget->text);
                    }
                }
                else if (nk_input_is_key_pressed(input, NK_KEY_UP)) {
                    widgetIndex--;
                    if (widgetIndex >= 0) {
                        console->activeWidget = parent->children[widgetIndex];
                        printf("Item: %s\n", console->activeWidget->text);
                    }
                }
            }
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

            nk_bool selected = top->activeWidget == console && top->selecting;


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
                            top->activeWidget = NULL;
                        }
                    }
                }
                else {
                    console->onclick(console);
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
    console->selecting = 0;
    console->user_data = NULL;
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