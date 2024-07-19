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
    nk_console_layout_widget(widget);

    struct nk_rect widget_bounds = nk_layout_widget_bounds(widget->ctx);

    if (widget->disabled || (widget->selectable && nk_console_get_active_widget(widget) != widget)) {
        nk_widget_disable_begin(widget->ctx);
    }

    nk_spacing(widget->ctx, widget->columns);

    if (widget->disabled || (widget->selectable && nk_console_get_active_widget(widget) != widget)) {
        nk_widget_disable_end(widget->ctx);
    }

    if (nk_console_is_active_widget(widget)) {
        nk_console_check_up_down(widget, widget_bounds);
        nk_console_check_tooltip(widget);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_spacing(nk_console* parent, int cols) {
    nk_console* spacing = nk_console_label(parent, NULL);
    spacing->type = NK_CONSOLE_SPACING;
    spacing->columns = cols;
    spacing->render = nk_console_spacing_render;
    return spacing;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_SPACING_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
