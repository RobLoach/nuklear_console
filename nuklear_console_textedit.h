#ifndef NK_CONSOLE_TEXTEDIT_H__
#define NK_CONSOLE_TEXTEDIT_H__

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct nk_console_textedit_data {
    nk_console_button_data button; // Inherits from button
    const char* buffer;
    int buffer_size;
} nk_console_textedit_data;

NK_API nk_console* nk_console_textedit(nk_console* parent, const char* label, const char* buffer, int buffer_size);
NK_API struct nk_rect nk_console_textedit_render(nk_console* widget);

NK_API const char* nk_console_textedit_get_buffer(nk_console* widget);
NK_API int nk_console_textedit_get_buffer_size(nk_console* widget);


#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_TEXTEDIT_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_TEXTEDIT_IMPLEMENTATION_ONCE
#define NK_CONSOLE_TEXTEDIT_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif


NK_API const char* nk_console_textedit_get_buffer(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) {
        return NULL;
    }
    nk_console_textedit_data* data = (nk_console_textedit_data*)widget->data;
    return data->buffer;
}

NK_API int nk_console_textedit_get_buffer_size(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) {
        return 0;
    }
    nk_console_textedit_data* data = (nk_console_textedit_data*)widget->data;
    return data->buffer_size;
}

NK_API void nk_console_textedit_onclick(nk_console* widget) {
    printf("Hello!\n");
}

NK_API struct nk_rect nk_console_textedit_render(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    // Start a new row.
    nk_layout_row_dynamic(widget->context, 0, nk_strlen(widget->label) > 0 ? 2 : 1);
    nk_console_textedit_data* data = (nk_console_textedit_data*)widget->data;

    // Display the label
    if (nk_strlen(widget->label) > 0) {
        if (widget->disabled || !nk_console_is_active_widget(widget)) {
            nk_widget_disable_begin(widget->context);
        }
        nk_console_label_render(widget);
        if (widget->disabled || !nk_console_is_active_widget(widget)) {
            nk_widget_disable_end(widget->context);
        }
    }

    // Display the button to open the textedit
    const char* label = widget->label;
    int label_length = widget->label_length;
    widget->label = nk_console_textedit_get_buffer(widget);
    if (widget->label == NULL) {
        widget->label = label;
        widget->label_length = label_length;
    }
    else {
        widget->label_length = nk_strlen(widget->label);
    }
    struct nk_rect size = nk_console_button_render(widget);
    widget->label = label;
    widget->label_length = label_length;

    return size;
}

NK_API nk_console* nk_console_textedit(nk_console* parent, const char* label, const char* buffer, int buffer_size) {
    // Create the widget data.
    nk_handle unused = {0};
    nk_console_textedit_data* data = (nk_console_textedit_data*)NK_CONSOLE_MALLOC(unused, NULL, sizeof(nk_console_textedit_data));
    nk_zero(data, sizeof(nk_console_textedit_data));
    data->buffer = buffer;
    data->buffer_size = buffer_size;

    nk_console* widget = nk_console_label(parent, label);
    widget->data = (void*)data;
    widget->type = NK_CONSOLE_TEXTEDIT;
    widget->render = nk_console_textedit_render;
    widget->selectable = nk_true;
    nk_console_button_set_onclick(widget, nk_console_textedit_onclick);
    widget->columns = 0;
    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_TEXTEDIT_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
