#ifndef NK_CONSOLE_KNOB_H__
#define NK_CONSOLE_KNOB_H__

#include "nuklear_console_property.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Data for Property and Slider widgets.
 */
typedef struct nk_console_knob_data {
    nk_console_property_data property; /* Inherited from property */
    enum nk_heading zero_direction;
    float dead_zone_degrees;
} nk_console_knob_data;

/**
 * Creates a Knob widget using an integer value.
 */
NK_API nk_console* nk_console_knob_int(nk_console* parent, const char* label, int min, int* val, int max, int step, float inc_per_pixel);

/**
 * Creates a Knob widget using a float value.
 */
NK_API nk_console* nk_console_knob_float(nk_console* parent, const char* label, float min, float* val, float max, float step, float inc_per_pixel);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_KNOB_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_KNOB_IMPLEMENTATION_ONCE
#define NK_CONSOLE_KNOB_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_knob_int(nk_console* parent, const char* label, int min, int* val, int max, int step, float inc_per_pixel) {
    // Create the property data.
    nk_console_knob_data* data = (nk_console_knob_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_knob_data));
    nk_zero(data, sizeof(nk_console_knob_data));
    data->property.min_int = min;
    data->property.val_int = val;
    data->property.max_int = max;
    data->property.step_int = step;
    data->property.inc_per_pixel = inc_per_pixel;

    data->zero_direction = NK_DOWN;
    data->dead_zone_degrees = 60.0f;

    nk_console* widget = nk_console_label(parent, label);
    widget->render = &nk_console_property_render;
    widget->type = NK_CONSOLE_KNOB_INT;
    widget->selectable = nk_true;
    widget->data = (void*)data;
    widget->columns = label != NULL ? 2 : 1;

    if (val != NULL) {
        if (*val < min) {
            *val = min;
        }
        else if (*val > max) {
            *val = max;
        }
    }

    return widget;
}

NK_API nk_console* nk_console_knob_float(nk_console* parent, const char* label, float min, float* val, float max, float step, float inc_per_pixel) {
    nk_console* widget = nk_console_knob_int(parent, label, 0, NULL, 0, 0, inc_per_pixel);
    nk_console_knob_data* data = (nk_console_knob_data*)widget->data;
    widget->type = NK_CONSOLE_KNOB_FLOAT;
    data->property.min_float = min;
    data->property.val_float = val;
    data->property.max_float = max;
    data->property.step_float = step;

    if (val != NULL) {
        if (*val < min) {
            *val = min;
        }
        else if (*val > max) {
            *val = max;
        }
    }

    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_KNOB_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
