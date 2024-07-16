#ifndef NK_CONSOLE_FILE_H__
#define NK_CONSOLE_FILE_H__

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef NK_CONSOLE_FILE_PATH_MAX
#ifdef PATH_MAX
#define NK_CONSOLE_FILE_PATH_MAX PATH_MAX
#else
#define NK_CONSOLE_FILE_PATH_MAX 1024
#endif
#endif  // NK_CONSOLE_FILE_PATH_MAX

typedef struct nk_console_file_data {
    nk_console_button_data button; // Inherited from button.
    char* file_path_buffer;
    int file_path_buffer_size;
    char directory[NK_CONSOLE_FILE_PATH_MAX];
} nk_console_file_data;

NK_API nk_console* nk_console_file(nk_console* parent, const char* label, char* file_path_buffer, int file_path_buffer_size);
NK_API struct nk_rect nk_console_file_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_FILE_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_FILE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_FILE_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

static const char* nk_console_file_basename(const char* path) {
    if (path == NULL) {
        return NULL;
    }
    int len = nk_strlen(path);
    for (int i = len - 1; i > 0; i--) {
        if(path[i] == '\\' || path[i] == '/' ){
            path = path + i + 1;
            break;
        }
    }
    return path;
}

NK_API struct nk_rect nk_console_file_render(nk_console* console) {
    if (console == NULL || console->data == NULL) {
        return nk_rect(0, 0, 0, 0);
    }
    nk_console_file_data* data = (nk_console_file_data*)console->data;
    //nk_console* top = nk_console_get_top(console);

    nk_console_layout_widget(console);

    // Display the label
    if (console->label != NULL && console->label[0] != '\0') {
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_begin(console->ctx);
        }
        if (console->label_length > 0) {
            nk_text(console->ctx, console->label, console->label_length, NK_TEXT_LEFT);
        }
        else {
            nk_label(console->ctx, console->label, NK_TEXT_LEFT);
        }
        if (!nk_console_is_active_widget(console)) {
            nk_widget_disable_end(console->ctx);
        }
    }

    // Display the mocked button
    int swap_columns = console->columns;
    const char* swap_label = console->label;
    console->columns = 0;
    console->label = nk_console_file_basename(data->file_path_buffer);
    struct nk_rect widget_bounds = nk_console_button_render(console);
    console->columns = swap_columns;
    console->label = swap_label;

    return widget_bounds;
}

/**
 * Gets the file widget from a child button.
 */
static nk_console* nk_console_file_button_get_file_widget(nk_console* button) {
    if (button == NULL) {
        return NULL;
    }
    if (button->type == NK_CONSOLE_FILE) {
        return button;
    }
    while (button->parent != NULL) {
        button = button->parent;
        if (button->type == NK_CONSOLE_FILE) {
            return button;
        }
    }

    return NULL;
}

/**
 * Button callback for the main file button.
 */
static void nk_console_file_main_click(nk_console* button) {
    if (button == NULL || button->data == NULL) {
        return;
    }

    nk_console* file = nk_console_file_button_get_file_widget(button);
    if (file == NULL || file->data == NULL) {
        return;
    }

    nk_console_file_data* data = (nk_console_file_data*)file->data;

    printf("Select a file\n");

    nk_console_show_message(button, "Select a file");

    NK_MEMCPY(data->file_path_buffer, "C:/Users/RobLoach/Documents/Projects/nuklear-console/nuklear_console_file.h", 75);
    data->file_path_buffer[75] = '\0';
}

NK_API nk_console* nk_console_file(nk_console* parent, const char* label, char* file_path_buffer, int file_path_buffer_size) {
    if (parent == NULL || file_path_buffer == NULL || file_path_buffer_size <= 0) {
        return NULL;
    }

    // Create the widget data.
    nk_handle unused = {0};
    nk_console_file_data* data = (nk_console_file_data*)NK_CONSOLE_MALLOC(unused, NULL, sizeof(nk_console_file_data));
    nk_zero(data, sizeof(nk_console_file_data));

    data->file_path_buffer = file_path_buffer;
    data->file_path_buffer_size = file_path_buffer_size;

    nk_console* widget = nk_console_label(parent, label);
    widget->type = NK_CONSOLE_FILE;
    widget->columns = label == NULL ? 1 : 2;
    widget->render = nk_console_file_render;
    widget->selectable = nk_true;
    widget->data = data;

    nk_console_button_set_onclick(widget, nk_console_file_main_click);
    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_FILE_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
