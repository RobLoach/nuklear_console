#ifndef NK_CONSOLE_SPACING_H__
#define NK_CONSOLE_SPACING_H__

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_spacing(nk_console* parent, int cols);
NK_API struct nk_rect nk_console_spacing_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_SPACING_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_SPACING_IMPLEMENTATION_ONCE
#define NK_CONSOLE_SPACING_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_spacing_render(nk_console* widget) {
    if (widget == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    // Set up the layout for the widget.
    nk_console_layout_widget(widget);

    // Render the spacing.
    nk_spacing(widget->ctx, widget->columns);

    return nk_rect(0, 0, 0, 0);
}

NK_API nk_console* nk_console_spacing(nk_console* parent, int cols) {
    nk_console* spacing = nk_console_label(parent, NULL);
    spacing->type = NK_CONSOLE_SPACING;
    spacing->columns = cols;
    spacing->render = nk_console_spacing_render;
    spacing->disabled = nk_true;
    spacing->selectable = nk_false;
    return spacing;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_SPACING_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
