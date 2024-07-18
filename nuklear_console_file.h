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
    nk_console_button_data button; /** Inherited from Button */
    char* file_path_buffer; /** The string buffer for the chosen file path. */
    int file_path_buffer_size; /** The size of the buffer. */
    char directory[NK_CONSOLE_FILE_PATH_MAX]; /** When selecting a file, this is the current directory. */
    void* file_user_data; /** Custom user data for the file system. */
} nk_console_file_data;

/**
 * Creates a file widget that allows the user to select a file.
 *
 * @param parent The parent widget.
 * @param label The label for the file widget. For example: "Select a file".
 * @param file_path_buffer The buffer to store the file path.
 * @param file_path_buffer_size The size of the buffer.
 */
NK_API nk_console* nk_console_file(nk_console* parent, const char* label, char* file_path_buffer, int file_path_buffer_size);
NK_API struct nk_rect nk_console_file_render(nk_console* widget);
NK_API void nk_console_file_set_file_user_data(nk_console* file, void* user_data);
NK_API void* nk_console_file_get_file_user_data(nk_console* file);

/**
 * Adds a file entry to the file widget.
 */
NK_API void nk_console_file_add_entry(nk_console* parent, const char* path, nk_bool is_directory);

/**
 * Refreshes the file widget to display the contents of the current directory.
 */
NK_API void nk_console_file_refresh(nk_console* widget);

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
    if (data->file_path_buffer != NULL && data->file_path_buffer[0] != '\0') {
        console->label = nk_console_file_basename(data->file_path_buffer);
    }
    else {
        console->label = "[Select a File]";
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
NK_API void nk_console_file_free_entry(nk_console* button) {
    if (button == NULL) {
        return;
    }

    if (button->label != NULL) {
        nk_console_mfree(nk_handle_id(0), (void*)button->label);
        button->label = NULL;
    }
}

NK_API void nk_console_file_entry_onclick(nk_console* button) {
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
    NK_MEMCPY(data->directory + len, (void*)button->label, nk_strlen(button->label) + 1);

    enum nk_symbol_type symbol = nk_console_button_get_symbol(button);
    switch (symbol) {
        case NK_SYMBOL_TRIANGLE_LEFT: // Back
        case NK_SYMBOL_TRIANGLE_RIGHT: // Folder
            nk_console_set_active_parent(file);
            nk_console_file_refresh(file);
        break;
        default:
        {
            // Copy the string to the file buffer.
            // TODO: Ensure UTF-8 compatibility.
            int desired_length = nk_strlen(data->directory);
            if (desired_length >= data->file_path_buffer_size) {
                NK_ASSERT(0); // File path is too long
            }
            else {
                NK_MEMCPY(data->file_path_buffer, data->directory, desired_length);
                data->file_path_buffer[desired_length] = '\0';
            }

            // Now that we selected a file, we can exit.
            nk_console_set_active_parent(file->parent);
        }
        break;
    }
}

NK_API void nk_console_file_add_entry(nk_console* parent, const char* path, nk_bool is_directory) {
    if (parent == NULL || path == NULL || path[0] == '\0') {
        return;
    }

    int len = nk_strlen(path);

    // Ignore the current directory.
    if (len == 1 && path[0] == '.') {
        return;
    }
    else if (len == 2 && path[0] == '.' && path[1] == '.') {
        // Ignore the parent directory.
        return;
    }

    // Add the button.
    nk_console* button = nk_console_button(parent, NULL);

    // Copy the path for the Label
    // TODO: file: Ensure UTF-8 compatibility.
    button->label = (const char*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(char) * (len + 1));
    button->destroy = nk_console_file_free_entry; // Use the button destructor to clear the label data.
    char* label = (char*)button->label;

    // Use the base name as the label.
    const char* basename = nk_console_file_basename(path);
    int basename_len = nk_strlen(basename);
    NK_MEMCPY(label, basename, basename_len);
    label[basename_len] = '\0';

    // Symbol
    if (is_directory == nk_true) {
        nk_console_button_set_symbol(button, NK_SYMBOL_TRIANGLE_RIGHT);
    }

    // Event
    nk_console_button_set_onclick(button, nk_console_file_entry_onclick);

}

// TODO: Allow disabling tinydir
#ifndef NK_CONSOLE_FILE_ADD_FILES
#ifdef NK_CONSOLE_ENABLE_TINYDIR
#ifndef NK_CONSOLE_FILE_ADD_FILES_TINYDIR_H
#define NK_CONSOLE_FILE_ADD_FILES_TINYDIR_H "vendor/tinydir/tinydir.h"
#endif  // NK_CONSOLE_FILE_ADD_FILES_TINYDIR_H
#ifndef NK_CONSOLE_FILE_ADD_FILES_TINYDIR_SKIP
#include NK_CONSOLE_FILE_ADD_FILES_TINYDIR_H
#endif  // NK_CONSOLE_FILE_ADD_FILES_TINYDIR_SKIP

/**
 * Destroy callback; Clear the tinydir memory.
 */
static void nk_console_file_destroy_tinydir(nk_console* console) {
    void* file_user_data = nk_console_file_get_file_user_data(console);
    if (file_user_data == NULL) {
        return;
    }

    // Clear the tinydir data.
    tinydir_close((tinydir_dir*)file_user_data);
    nk_console_mfree(nk_handle_id(0), file_user_data);
    nk_console_file_set_file_user_data(console, NULL);
}

/**
 * Iterate through the files in the given directory, and add the contents  with nk_console_file_add_directory and nk_console_file_add_file.
 */
static void nk_console_file_add_files_tinydir(nk_console* parent, const char* directory) {
    if (parent == NULL || directory == NULL) {
        return;
    }

    // Initialize the tinydir memory if needed.
    tinydir_dir* dir = (tinydir_dir*)nk_console_file_get_file_user_data(parent);
    if (dir == NULL) {
        dir = NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, sizeof(tinydir_dir));
        if (dir == NULL) {
            return;
        }
        nk_console_file_set_file_user_data(parent, dir);

        // Use the destructor to close tinydir.
        parent->destroy = nk_console_file_destroy_tinydir;
    }
    else {
        // Since tinydir exists already, close the active directory.
        tinydir_close(dir);
    }

    // Open the new directory
    if (tinydir_open_sorted(dir, directory) == -1) {
        return;
    }

    // Iterate through the files and add each entry.
    for (size_t i = 0; i < dir->n_files; i++) {
        tinydir_file file;
        tinydir_readfile_n(dir, &file, i);
        nk_console_file_add_entry(parent, file.name, file.is_dir == 0 ? nk_false : nk_true);
    }
}
#define NK_CONSOLE_FILE_ADD_FILES nk_console_file_add_files_tinydir

#else  // NK_CONSOLE_ENABLE_TINYDIR

static void nk_console_file_add_files_diabled(nk_console* parent, const char* directory) {
    NK_UNUSED(directory);
    if (parent == NULL) {
        return;
    }

    // Requires NK_CONSOLE_ENABLE_TINYDIR or another file system library.
    nk_console_show_message(parent, "Error: File system not available.");
    parent->disabled = nk_true;
    nk_console_button_back(parent);
}
#define NK_CONSOLE_FILE_ADD_FILES nk_console_file_add_files_diabled

#endif  // NK_CONSOLE_ENABLE_TINYDIR
#endif  // NK_CONSOLE_FILE_ADD_FILES

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
NK_API void nk_console_file_refresh(nk_console* widget) {
    widget = nk_console_file_button_get_file_widget(widget);
    if (widget == NULL || widget->data == NULL) {
        return;
    }

    nk_console_file_data* data = (nk_console_file_data*)widget->data;

    // Clear out all the current entries.
    nk_console_free_children(widget);

    // Add the back/cancel button
    nk_console_button_onclick(widget, "Cancel", nk_console_button_back);

    // Active directory label
    nk_console_label(widget, data->directory)->alignment = NK_TEXT_CENTERED;

    // Add the parent directory button
    nk_console* parent_directory_button = nk_console_button_onclick(widget, "..", nk_console_file_entry_onclick);
    nk_console_button_set_symbol(parent_directory_button, NK_SYMBOL_TRIANGLE_LEFT);

    // Iterate through the files in the directory, and add them as entries.
    NK_CONSOLE_FILE_ADD_FILES(widget, data->directory);
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

    int directory_len = nk_console_file_get_directory_len(data->file_path_buffer);
    NK_MEMCPY(data->directory, data->file_path_buffer, directory_len);
    data->directory[directory_len] = '\0';

    if (nk_strlen(data->directory) == 0) {
        // TODO: file: Make get current working directory function.
        data->directory[0] = '.';
        data->directory[1] = '\0';
    }

    nk_console_set_active_parent(file);
    nk_console_file_refresh(file);
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
