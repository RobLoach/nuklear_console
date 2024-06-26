#ifndef NK_CONSOLE_LABEL_H__
#define NK_CONSOLE_LABEL_H__

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_label(nk_console* parent, const char* text);
NK_API struct nk_rect nk_console_label_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_LABEL_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_LABEL_IMPLEMENTATION_ONCE
#define NK_CONSOLE_LABEL_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_label_render(nk_console* widget) {
    if (nk_strlen(widget->label) <= 0) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(widget);

    // TODO: Add label options like alignment or text wrapping
    nk_label(widget->ctx, widget->label, widget->alignment);

    return nk_rect(0, 0, 0, 0);
}

NK_API nk_console* nk_console_label(nk_console* parent, const char* text) {
    nk_console* label = nk_console_init(parent->ctx);
    label->type = NK_CONSOLE_LABEL;
    label->label = text;
    label->parent = parent;
    label->alignment = NK_TEXT_LEFT;
    label->columns = 1;
    label->render = nk_console_label_render;
    cvector_push_back(parent->children, label);
    return label;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_LABEL_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
