#ifndef NK_CONSOLE_CHART_H__
#define NK_CONSOLE_CHART_H__

typedef struct nk_console_chart_data {
    enum nk_chart_type type;
    const float* values;
    int count;
    float min_value;
    float max_value;
} nk_console_chart_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Add a chart widget displaying @p values of the given @p type, scaled between @p min_value and @p max_value.
 *
 * When @p label is provided, it is displayed in a column next to the chart.
 * Use nk_console_set_height() to make the chart taller than a regular row.
 *
 * @return The new widget.
 */
NK_API nk_console* nk_console_chart(nk_console* parent, const char* label, enum nk_chart_type type, const float* values, int count, float min_value, float max_value);
/** Update the data of an existing chart widget. */
NK_API void nk_console_chart_update(nk_console* chart, const float* values, int count, float min_value, float max_value);
/** Render the chart widget. @return The bounding rect. */
NK_API struct nk_rect nk_console_chart_render(nk_console* console);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_CHART_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_CHART_IMPLEMENTATION_ONCE
#define NK_CONSOLE_CHART_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API struct nk_rect nk_console_chart_render(nk_console* console) {
    nk_console_chart_data* data = (nk_console_chart_data*)console->data;
    if (data == NULL || data->values == NULL || data->count <= 0) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(console);

    // Display the label when provided.
    if (console->label != NULL && nk_strlen(console->label) > 0) {
        nk_label(console->ctx, console->label, NK_TEXT_LEFT);
    }

    struct nk_rect widget_bounds = nk_layout_widget_bounds(console->ctx);

    if (console->disabled) {
        nk_widget_disable_begin(console->ctx);
    }

    // Use the given range so the chart doesn't auto-scale to its values.
    if (nk_chart_begin(console->ctx, data->type, data->count, data->min_value, data->max_value)) {
        for (int i = 0; i < data->count; i++) {
            nk_chart_push(console->ctx, data->values[i]);
        }
        nk_chart_end(console->ctx);
    }

    if (console->disabled) {
        nk_widget_disable_end(console->ctx);
    }

    if (nk_console_is_active_widget(console)) {
        nk_console_check_up_down(console);
        nk_console_check_tooltip(console);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_chart(nk_console* parent, const char* label, enum nk_chart_type type, const float* values, int count, float min_value, float max_value) {
    NK_ASSERT(parent != NULL);
    NK_ASSERT(values != NULL);
    NK_ASSERT(count > 0);

    nk_console_chart_data* data = (nk_console_chart_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_chart_data));
    nk_zero(data, sizeof(nk_console_chart_data));
    data->type = type;
    data->values = values;
    data->count = count;
    data->min_value = min_value;
    data->max_value = max_value;

    nk_console* chart = nk_console_label(parent, label);
    chart->render = nk_console_chart_render;
    chart->type = NK_CONSOLE_CHART;
    chart->selectable = nk_false;
    chart->columns = label != NULL ? 2 : 1;
    chart->data = (void*)data;
    return chart;
}

NK_API void nk_console_chart_update(nk_console* chart, const float* values, int count, float min_value, float max_value) {
    if (chart == NULL || chart->data == NULL) return;
    nk_console_chart_data* data = (nk_console_chart_data*)chart->data;
    data->values = values;
    data->count = count;
    data->min_value = min_value;
    data->max_value = max_value;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_CHART_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
