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

#endif // NK_CONSOLE_LABEL_H__

#ifdef NK_CONSOLE_IMPLEMENTATION
#ifndef NK_CONSOLE_LABEL_IMPLEMENTATION_ONCE
#define NK_CONSOLE_LABEL_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_label_render(nk_console* widget) {
    if (nk_strlen(widget->text) <= 0) {
        return nk_rect(0, 0, 0, 0);
    }

    if (widget->columns > 0) {
        nk_layout_row_dynamic(widget->context, 0, widget->columns);
    }

    // TODO: Add label options like alignment or text wrapping
    nk_label(widget->context, widget->text, widget->alignment);

    return nk_rect(0, 0, 0, 0);
}

NK_API nk_console* nk_console_label(nk_console* parent, const char* text) {
    nk_console* label = nk_console_init(parent->context);
    label->type = NK_CONSOLE_LABEL;
    label->text = text;
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

#endif
#endif
