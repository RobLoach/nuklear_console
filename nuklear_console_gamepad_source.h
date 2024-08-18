#ifndef NK_CONSOLE_GAMEPAD_SOURCE_H__
#define NK_CONSOLE_GAMEPAD_SOURCE_H__

/**
 * Data for the Gamepad Input Source combobox.
 */
typedef struct nk_console_gamepad_source_data {
    struct nk_gamepad_input_source* input_sources;
    char input_source_names[1024];
    int input_source_selected;
} nk_console_gamepad_source_data;

#if defined(__cplusplus)
extern "C" {
#endif

NK_API nk_console* nk_console_gamepad_source(nk_console* parent, const char* label, struct nk_gamepad_input_source* input_sources, int input_sources_count);
NK_API void nk_console_gamepad_source_changed(nk_console* widget, void* user_data);
NK_API void nk_console_gamepad_source_destroyed(nk_console* widget, void* user_data);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_GAMEPAD_SOURCE_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_GAMEPAD_SOURCE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_GAMEPAD_SOURCE_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API void nk_console_gamepad_source_changed(nk_console* widget, void* user_data) {
    if (widget == NULL || user_data == NULL) {
        return;
    }

    nk_console_gamepad_source_data* data = (nk_console_gamepad_source_data*)user_data;
    if (data->input_source_selected < 0 || (size_t)data->input_source_selected >= cvector_size(data->input_sources)) {
        return;
    }

    struct nk_gamepad_input_source input_source = data->input_sources[data->input_source_selected];
    nk_console* top = nk_console_get_top(widget);
    if (top == NULL || top->data == NULL) {
        return;
    }

    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    if (nk_gamepad_set_input_source(top_data->gamepads, input_source) == nk_false) {
        nk_console_show_message(top, "Failed to set gamepad input source");
    };
}

NK_API void nk_console_gamepad_source_destroyed(nk_console* widget, void* user_data) {
    if (widget == NULL || user_data == NULL) {
        return;
    }
    nk_console_gamepad_source_data* data = (nk_console_gamepad_source_data*)user_data;
    cvector_free(data->input_sources);
}

#ifndef NK_STRCAT
#define NK_STRCAT nk_console_gamepad_source_strcat
static char* nk_console_gamepad_source_strcat(char *dest, const char *src) {
    size_t i, j;
    for (i = 0; dest[i] != '\0'; i++);
    for (j = 0; src[j] != '\0'; j++) {
        dest[i + j] = src[j];
    }
    dest[i + j] = '\0';
    return dest;
}
#endif

NK_API nk_console* nk_console_gamepad_source(nk_console* parent, const char* label, struct nk_gamepad_input_source* input_sources, int input_sources_count) {
    if (parent == NULL || input_sources_count <= 0) {
        return NULL;
    }

    nk_console_gamepad_source_data* data = (nk_console_gamepad_source_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_gamepad_source_data));
    if (data == NULL) {
        return NULL;
    }
    nk_zero(data, sizeof(nk_console_gamepad_source_data));

    // Add all input sources to the list.
    for (int i = 0; i < input_sources_count; i++) {
        if (nk_strlen(input_sources[i].input_source_name) == 0) {
            continue;
        }

        // Add the input source to the list.
        cvector_push_back(data->input_sources, input_sources[i]);

        // Add the input source name to the list.
        NK_STRCAT(data->input_source_names, input_sources[i].input_source_name);

        // Only concat a seperator if there are more items.
        if (i < input_sources_count - 1) {
            NK_STRCAT(data->input_source_names, ";");
        }
    }

    // Build the combobox
    nk_console* widget = nk_console_combobox(parent, label, data->input_source_names, ';', &data->input_source_selected);
    if (widget == NULL) {
        return NULL;
    }

    nk_console_add_event_handler(widget, NK_CONSOLE_EVENT_CHANGED, nk_console_gamepad_source_changed, data, nk_console_gamepad_source_destroyed);

    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_GAMEPAD_SOURCE_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
