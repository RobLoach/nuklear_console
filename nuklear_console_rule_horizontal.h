#ifndef NK_CONSOLE_RULE_HORIZONTAL_H__
#define NK_CONSOLE_RULE_HORIZONTAL_H__

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Data for the horizontal rule widget.
 *
 * @see nk_console_rule_horizontal()
 */
typedef struct nk_console_rule_horizontal_data {
    struct nk_color color;
    nk_bool rounding;
} nk_console_rule_horizontal_data;

/**
 * Line for visual separation. Draws a line with thickness determined by the current row height.
 */
NK_API nk_console* nk_console_rule_horizontal(nk_console* parent, struct nk_color color, nk_bool rounding);

/**
 * Renders the Horizontal Rule.
 */
NK_API struct nk_rect nk_console_rule_horizontal_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_RULE_HORIZONTAL_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_RULE_HORIZONTAL_IMPLEMENTATION_ONCE
#define NK_CONSOLE_RULE_HORIZONTAL_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_rule_horizontal_render(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }
    nk_console_rule_horizontal_data* data = (nk_console_rule_horizontal_data*)widget->data;
    struct nk_rect widget_bounds = nk_layout_widget_bounds(widget->ctx);
    nk_rule_horizontal(widget->ctx, data->color, data->rounding);

    return widget_bounds;
}

NK_API nk_console* nk_console_rule_horizontal(nk_console* parent, struct nk_color color, nk_bool rounding) {
    if (parent == NULL) {
        return NULL;
    }
    nk_console_rule_horizontal_data* data =
        (nk_console_rule_horizontal_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_rule_horizontal_data));
    data->color = color;
    data->rounding = rounding;

    nk_console* rule = nk_console_label(parent, NULL);
    rule->type = NK_CONSOLE_RULE_HORIZONTAL;
    rule->alignment = NK_TEXT_CENTERED;
    rule->data = (void*)data;
    rule->render = nk_console_rule_horizontal_render;
    rule->disabled = nk_true;
    rule->selectable = nk_false;
    return rule;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_RULE_HORIZONTAL_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
