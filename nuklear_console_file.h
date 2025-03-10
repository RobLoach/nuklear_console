#ifndef NK_CONSOLE_FILE_H__
#define NK_CONSOLE_FILE_H__

#ifndef NK_CONSOLE_FILE_PATH_MAX
#ifdef PATH_MAX
#define NK_CONSOLE_FILE_PATH_MAX PATH_MAX
#else
#define NK_CONSOLE_FILE_PATH_MAX 4096
#endif
#endif // NK_CONSOLE_FILE_PATH_MAX

/**
 * Custom data for the file widget.
 */
typedef struct nk_console_file_data {
    nk_console_button_data button; /** Inherited from Button */
    char* file_path_buffer; /** The string buffer for the chosen file path. */
    int file_path_buffer_size; /** The size of the buffer. */
    char directory[NK_CONSOLE_FILE_PATH_MAX]; /** When selecting a file, this is the current directory. */
    void* file_user_data; /** Custom user data for the file system. */
    nk_bool select_directory; /** Flag indicating if we are selecting a directory. */
} nk_console_file_data;

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Creates a file widget that allows the user to select a file.
 *
 * @param parent The parent widget.
 * @param label The label for the file widget. For example: "Select a file".
 * @param file_path_buffer The buffer to store the file path.
 * @param file_path_buffer_size The size of the buffer.
 *
 * @return The new file widget.
 */
NK_API nk_console* nk_console_file(nk_console* parent, const char* label, char* file_path_buffer, int file_path_buffer_size);

/**
 * Creates a directory widget that allows the user to select a directory.
 *
 * @param parent The parent widget.
 * @param label The label for the file widget. For example: "Select a file".
 * @param dir_buffer The buffer to store the file path.
 * @param dir_buffer_size The size of the buffer.
 *
 * @return The new file widget.
 */
NK_API nk_console* nk_console_dir(nk_console* parent, const char* label, char* dir_buffer, int dir_buffer_size);

/**
 * Render callback to display the file widget.
 */
NK_API struct nk_rect nk_console_file_render(nk_console* widget);

/**
 * Sets any custom user data specifically for the file widget.
 *
 * This can be helpful for storing additional file system data for the file widget.
 *
 * @param file The file widget.
 * @param user_data The custom user data.
 */
NK_API void nk_console_file_set_file_user_data(nk_console* file, void* user_data);

/**
 * Gets the custom user data specifically for the file widget.
 *
 * @param file The file widget.
 *
 * @return The custom user data.
 */
NK_API void* nk_console_file_get_file_user_data(nk_console* file);

/**
 * Add a individual file or directory to the given file widget as a child.
 *
 * This should be called from the file system callbacks. See `nuklear_console_file_system.h` for examples.
 *
 * @param parent The file widget.
 * @param path The path to the file or directory.
 * @param is_directory True if the path is a directory. False otherwise.
 *
 * @return The new button if it was successfully added, NULL otherwise.
 *
 * @see nk_console_file_destroy_tinydir()
 * @see nk_console_file_add_files_raylib()
 */
NK_API nk_console* nk_console_file_add_entry(nk_console* parent, const char* path, nk_bool is_directory);

/**
 * Refreshes the file widget to display the contents of the current directory.
 *
 * @param widget The file widget to refresh.
 *
 * @see nk_console_file_data::directory
 */
NK_API void nk_console_file_refresh(nk_console* widget, void* user_data);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_FILE_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_FILE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_FILE_IMPLEMENTATION_ONCE

#include "nuklear_console_file_system.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Gets the base name of a file path.
 */
static const char* nk_console_file_basename(const char* path) {
    if (path == NULL) {
        return NULL;
    }

    // TODO: Ensure UTF-8 compatibility.
    int len = nk_strlen(path);
    for (int i = len - 1; i > 0; i--) {
        if (path[i] == '\\' || path[i] == '/') {
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
    if (data->file_path_buffer != NULL && data->file_path_buffer[0] != '\0') {
        console->label = nk_console_file_basename(data->file_path_buffer);
    }
    else {
        console->label = data->select_directory ? "[Select Directory]" : "[Select a File]";
    }
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
 * Free the individual file entry buttons. This clears the label.
 */
NK_API void nk_console_file_free_entry(nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    if (button == NULL) {
        return;
    }

    if (button->label != NULL) {
        nk_console_mfree(nk_handle_id(0), (void*)button->label);
        button->label = NULL;
    }
}

NK_API void nk_console_file_entry_onclick(nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    if (button == NULL || button->label == NULL) {
        return;
    }

    nk_console* file = nk_console_file_button_get_file_widget(button);
    if (file == NULL || file->data == NULL) {
        return;
    }

    nk_console_file_data* data = (nk_console_file_data*)file->data;
    int len = nk_strlen(data->directory);

    // Append a slash if the directory is not empty.
    if (len == 1 && data->directory[0] == '.') {
        len = 0;
        data->directory[0] = '\0';
    }
    else if (len > 0) {
// TODO: file: Make sure this is cross-platform.
#if defined(_WIN32) || defined(WIN32)
        data->directory[len] = '\\';
#else
        data->directory[len] = '/';
#endif
        data->directory[len + 1] = '\0';
        len++;
    }

    // Concatenate the button label to the directory.
    // TODO: file: Resolve the path properly, so the paths don't recurse. For example: folder/../folder
    // TODO: file: Add UTF-8 support.
    NK_MEMCPY(data->directory + len, (void*)button->label, (nk_size)(nk_strlen(button->label) + 1));

    enum nk_symbol_type symbol = nk_console_button_get_symbol(button);
    switch (symbol) { // Directory
        case NK_SYMBOL_TRIANGLE_LEFT: // Back
        case NK_SYMBOL_TRIANGLE_RIGHT: // Folder
            nk_console_set_active_parent(file);
            nk_console_add_event(file, NK_CONSOLE_EVENT_POST_RENDER_ONCE, &nk_console_file_refresh);
            break;
        default: // File
        {
            // Copy the string to the file buffer.
            // TODO: Ensure UTF-8 compatibility.
            int desired_length = nk_strlen(data->directory);
            if (desired_length >= data->file_path_buffer_size) {
                NK_ASSERT(0); // File path is too long
                nk_console_show_message(file, "Error: File path is too long.");
            }
            else {
                NK_MEMCPY(data->file_path_buffer, data->directory, (nk_size)desired_length);
                data->file_path_buffer[desired_length] = '\0';

                // Trigger the onchange event and exit.
                nk_console_trigger_event(file, NK_CONSOLE_EVENT_CHANGED);
            }

            // Now that we selected a file, we can exit.
            nk_console_set_active_parent(file->parent);
        } break;
    }
}

NK_API nk_console* nk_console_file_add_entry(nk_console* parent, const char* path, nk_bool is_directory) {
    if (parent == NULL || path == NULL || path[0] == '\0' || parent->data == NULL) {
        return NULL;
    }

    // Are we only selecting directories?
    nk_console_file_data* data = (nk_console_file_data*)nk_console_file_button_get_file_widget(parent)->data;
    if (is_directory == nk_false && data->select_directory == nk_true) {
        return NULL;
    }

    int len = nk_strlen(path);

    // Ignore the current directory.
    if (len == 1 && path[0] == '.') {
        return NULL;
    }
    else if (len == 2 && path[0] == '.' && path[1] == '.') {
        // Ignore the parent directory.
        return NULL;
    }

    // Add the button.
    nk_console* button = nk_console_button(parent, NULL);

    // Copy the path for the label, and register an event to destroy it.
    // TODO: file: Ensure UTF-8 compatibility.
    button->label = (const char*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, (nk_size)(sizeof(char)) * (nk_size)(len + 1));
    nk_console_add_event(button, NK_CONSOLE_EVENT_DESTROYED, &nk_console_file_free_entry);

    char* label = (char*)button->label;

    // Use the base name as the label.
    const char* basename = nk_console_file_basename(path);
    nk_size basename_len = (nk_size)nk_strlen(basename);
    NK_MEMCPY(label, basename, basename_len);
    label[basename_len] = '\0';

    // Symbol
    if (is_directory == nk_true) {
        nk_console_button_set_symbol(button, NK_SYMBOL_TRIANGLE_RIGHT);
    }

    // Event
    nk_console_add_event(button, NK_CONSOLE_EVENT_CLICKED, &nk_console_file_entry_onclick);
    return button;
}


/**
 * Gets the length of the directory string of the given file path.
 */
static int nk_console_file_get_directory_len(const char* file_path) {
    if (file_path == NULL) {
        return 0;
    }
    int len = nk_strlen(file_path);
    for (int i = len - 1; i > 0; i--) {
        if (file_path[i] == '\\' || file_path[i] == '/') {
            return i;
        }
    }
    return 0;
}

/**
 * Fills the files array with the files in the current directory.
 */
NK_API void nk_console_file_refresh(nk_console* widget, void* user_data) {
    NK_UNUSED(user_data);
    widget = nk_console_file_button_get_file_widget(widget);
    if (widget == NULL || widget->data == NULL) {
        return;
    }

    nk_console_file_data* data = (nk_console_file_data*)widget->data;

    // Clear out all the current entries.
    nk_console_free_children(widget);

    // Add the back/cancel button
    nk_console* cancelButton = nk_console_button_onclick(widget, "Cancel", &nk_console_button_back);
    nk_console_button_set_symbol(cancelButton, NK_SYMBOL_X);

    // Show the Active directory.
    if (!data->select_directory) {
        // Active directory label
        nk_console* activeLabel = nk_console_label(widget, data->directory);
        activeLabel->alignment = NK_TEXT_CENTERED;
    }
    else {
        // Add the select directory button.
        nk_console* button = nk_console_file_add_entry(widget, data->directory, nk_true);
        nk_console_button_set_symbol(button, NK_SYMBOL_CIRCLE_SOLID);
        nk_console_set_tooltip(button, "Use this directory");
    }

    // Add the parent directory button
    nk_console* parent_directory_button = nk_console_button_onclick(widget, "..", &nk_console_file_entry_onclick);
    nk_console_button_set_symbol(parent_directory_button, NK_SYMBOL_TRIANGLE_LEFT);
    nk_console_set_tooltip(parent_directory_button, "Navigate to the parent directory");
    nk_console_set_active_widget(parent_directory_button);

#ifdef NK_CONSOLE_FILE_ADD_FILES
    // Iterate through the files in the directory, and add them as entries.
    if (NK_CONSOLE_FILE_ADD_FILES(widget, data->directory) == nk_false) {
        nk_console_label(widget, "No files found.")->alignment = NK_TEXT_CENTERED;
    }
#else
    // NK_CONSOLE_FILE_ADD_FILES is undefined, so back out.
    nk_console_show_message(widget, "Error: File system not available.");

    // Go back to the parent widget, and disable the widget.
    if (widget->parent != NULL) {
        widget->disabled = nk_true;
        nk_console_set_active_parent(widget->parent);
    }
#endif
}

/**
 * Button callback for the main file button.
 */
static void nk_console_file_main_click(nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    if (button == NULL || button->data == NULL) {
        return;
    }

    nk_console* file = nk_console_file_button_get_file_widget(button);
    if (file == NULL || file->data == NULL) {
        return;
    }

    nk_console_file_data* data = (nk_console_file_data*)file->data;

    int directory_len = nk_console_file_get_directory_len(data->file_path_buffer);
    NK_MEMCPY(data->directory, data->file_path_buffer, (nk_size)directory_len);
    data->directory[directory_len] = '\0';

    if (nk_strlen(data->directory) == 0) {
        // TODO: file: Make get current working directory function.
        data->directory[0] = '.';
        data->directory[1] = '\0';
    }

    // Set the active parent to the file widget, and refresh it after rendering everything else.
    nk_console_set_active_parent(file);
    nk_console_add_event(file, NK_CONSOLE_EVENT_POST_RENDER_ONCE, &nk_console_file_refresh);
}

NK_API void nk_console_file_set_file_user_data(nk_console* file, void* user_data) {
    file = nk_console_file_button_get_file_widget(file);
    if (file == NULL || file->data == NULL) {
        return;
    }
    nk_console_file_data* data = (nk_console_file_data*)file->data;
    data->file_user_data = user_data;
}

NK_API void* nk_console_file_get_file_user_data(nk_console* file) {
    file = nk_console_file_button_get_file_widget(file);
    if (file == NULL || file->data == NULL) {
        return NULL;
    }
    nk_console_file_data* data = (nk_console_file_data*)file->data;
    return data->file_user_data;
}

NK_API nk_console* nk_console_file(nk_console* parent, const char* label, char* file_path_buffer, int file_path_buffer_size) {
    if (parent == NULL || file_path_buffer == NULL || file_path_buffer_size <= 0) {
        return NULL;
    }

    // Create the widget data.
    nk_console_file_data* data = (nk_console_file_data*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(nk_console_file_data));
    nk_zero(data, sizeof(nk_console_file_data));

    data->file_path_buffer = file_path_buffer;
    data->file_path_buffer_size = file_path_buffer_size;

    nk_console* widget = nk_console_label(parent, label);
    widget->type = NK_CONSOLE_FILE;
    widget->columns = label == NULL ? 1 : 2;
    widget->render = nk_console_file_render;
    widget->selectable = nk_true;
    widget->data = data;

    nk_console_add_event(widget, NK_CONSOLE_EVENT_CLICKED, &nk_console_file_main_click);
    return widget;
}

NK_API nk_console* nk_console_dir(nk_console* parent, const char* label, char* dir_buffer, int dir_buffer_size) {
    nk_console* widget = nk_console_file(parent, label, dir_buffer, dir_buffer_size);
    if (widget == NULL) {
        return NULL;
    }

    nk_console_file_data* data = (nk_console_file_data*)widget->data;
    data->select_directory = nk_true;

    return widget;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_FILE_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
