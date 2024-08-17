#ifndef NK_CONSOLE_IMAGE_H__
#define NK_CONSOLE_IMAGE_H__

typedef struct nk_console_image_data {
    struct nk_image image;
    struct nk_color color;
} nk_console_image_data;

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_image(nk_console* parent, struct nk_image image);
NK_API nk_console* nk_console_image_color(nk_console* parent, struct nk_image image, struct nk_color color);
NK_API struct nk_rect nk_console_image_render(nk_console* widget);

NK_API void nk_console_image_set_image(nk_console* widget, struct nk_image image);
NK_API struct nk_image nk_console_image_get_image(nk_console* widget);

NK_API void nk_console_image_set_color(nk_console* widget, struct nk_color color);
NK_API struct nk_color nk_console_image_get_color(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_IMAGE_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_IMAGE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_IMAGE_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API void nk_console_image_set_image(nk_console* widget, struct nk_image image) {
    if (widget == NULL || widget->data == NULL) {
        return;
    }
    nk_console_image_data* data = (nk_console_image_data*)widget->data;
    data->image = image;
}

NK_API struct nk_image nk_console_image_get_image(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) {
        struct nk_image output = {0};
        return output;
    }
    nk_console_image_data* data = (nk_console_image_data*)widget->data;
    return data->image;
}

NK_API void nk_console_image_set_color(nk_console* widget, struct nk_color color) {
    if (widget == NULL || widget->data == NULL) {
        return;
    }
    nk_console_image_data* data = (nk_console_image_data*)widget->data;
    data->color = color;
}

NK_API struct nk_color nk_console_image_get_color(nk_console* widget) {
    if (widget == NULL || widget->data == NULL) {
        struct nk_color output = {0};
        return output;
    }
    nk_console_image_data* data = (nk_console_image_data*)widget->data;
    return data->color;
}

NK_API struct nk_rect nk_console_image_render(nk_console* widget) {
    nk_console_image_data* data = (nk_console_image_data*)widget->data;
    if (data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }

    nk_console_layout_widget(widget);

    struct nk_rect widget_bounds = nk_layout_widget_bounds(widget->ctx);

    // Toggle it as disabled if needed.
    if (widget->disabled ||
        (widget->selectable && nk_console_get_active_widget(widget) != widget)) {
        nk_widget_disable_begin(widget->ctx);
    }

    nk_image_color(widget->ctx, data->image, data->color);

    // Release the disabled state if needed.
    if (widget->disabled ||
        (widget->selectable && nk_console_get_active_widget(widget) != widget)) {
        nk_widget_disable_end(widget->ctx);
    }

    if (nk_console_is_active_widget(widget)) {
        nk_console_check_up_down(widget, widget_bounds);
        nk_console_check_tooltip(widget);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_image(nk_console* parent, struct nk_image image) {
    struct nk_color white = {255, 255, 255, 255};
    return nk_console_image_color(parent, image, white);
}

NK_API
nk_console* nk_console_image_color(nk_console* parent, struct nk_image image, struct nk_color color) {
    nk_console_image_data* data =
        (nk_console_image_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(*data));
    data->image = image;
    data->color = color;

    nk_console* img = nk_console_label(parent, NULL);
    img->type = NK_CONSOLE_IMAGE;
    img->data = (void*)data;
    img->render = nk_console_image_render;
    return img;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_IMAGE_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
