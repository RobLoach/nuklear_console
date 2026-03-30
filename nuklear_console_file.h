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
 * A single file or directory entry stored for the file widget's list view.
 */
typedef struct nk_console_file_entry {
    char* label;         /** The basename of the file or directory. */
    nk_bool is_directory; /** True if this entry is a directory. */
} nk_console_file_entry;

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
    char dir_label_buf[NK_CONSOLE_FILE_PATH_MAX + 2]; /** Scratch buffer for appending "/" to directory labels in the list view. */
    nk_console_file_entry* entries; /** cvector of file/directory entries for the list view. */
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
 * @param label The label for the file widget. For example: "Select a directory".
 * @param dir_buffer The buffer to store the directory path.
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
 * Add an individual file or directory entry to the given file widget's list view.
 *
 * This should be called from the file system callbacks. See `nuklear_console_file_system.h` for examples.
 *
 * @param parent The file widget.
 * @param path The path to the file or directory.
 * @param is_directory True if the path is a directory. False otherwise.
 *
 * @return Non-NULL if the entry was successfully added, NULL otherwise.
 *
 * @see nk_console_file_add_files_tinydir()
 * @see nk_console_file_add_files_raylib()
 */
NK_API nk_console* nk_console_file_add_entry(nk_console* parent, const char* path, nk_bool is_directory);

/**
 * Refreshes the file widget with the contents with its given directory.
 *
 * @internal
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
 *
 * @internal
 */
static const char* nk_console_file_basename(const char* path) {
    if (path == NULL) {
        return NULL;
    }

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
 *
 * @internal
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
 * Free all file entries stored in the file data.
 *
 * @internal
 */
static void nk_console_file_entries_clear(nk_console_file_data* data) {
    if (data == NULL) {
        return;
    }
    for (size_t i = 0; i < cvector_size(data->entries); i++) {
        if (data->entries[i].label != NULL) {
            nk_console_mfree(nk_handle_id(0), data->entries[i].label);
            data->entries[i].label = NULL;
        }
    }
    cvector_clear(data->entries);
}

/**
 * Event handler: Destroy the file widget.
 *
 * @internal
 */
static void nk_console_file_event_destroy(nk_console* file, void* user_data) {
    NK_UNUSED(user_data);
    if (file == NULL || file->data == NULL) {
        return;
    }
    nk_console_file_data* data = (nk_console_file_data*)file->data;

    // Clear all the file entries.
    nk_console_file_entries_clear(data);
    cvector_free(data->entries);
    data->entries = NULL;
}

#ifdef NK_CONSOLE_FILE_ADD_FILES
/**
 * get_label callback for the file list view. Appends "/" to directory entries.
 *
 * @internal
 */
static const char* nk_console_file_list_view_get_label(struct nk_console* list_view, nk_uint index) {
    nk_console* file = nk_console_file_button_get_file_widget(list_view);
    if (file == NULL || file->data == NULL) {
        return NULL;
    }
    nk_console_file_data* data = (nk_console_file_data*)file->data;
    if (cvector_empty(data->entries)) {
        return "(Empty directory)";
    }
    if (index >= cvector_size(data->entries)) {
        return NULL;
    }
    nk_console_file_entry* entry = &data->entries[index];
    if (entry->is_directory) {
        nk_size len = (nk_size)nk_strlen(entry->label);
        if (len + 2 > sizeof(data->dir_label_buf)) {
            return entry->label;
        }
        NK_MEMCPY(data->dir_label_buf, entry->label, len);
        data->dir_label_buf[len] = '/';
        data->dir_label_buf[len + 1] = '\0';
        return data->dir_label_buf;
    }
    return entry->label;
}
#endif

/**
 * Appends a path component to data->directory, inserting the platform separator.
 * Returns nk_false and shows an error if the result would overflow the buffer.
 *
 * @internal
 */
static nk_bool nk_console_file_append_to_directory(nk_console_file_data* data, nk_console* file, const char* label) {
    int len = nk_strlen(data->directory);
    int label_len = nk_strlen(label);

    int dir_len_after_slash = (len == 1 && data->directory[0] == '.') ? 0 : (len > 0 ? len + 1 : 0);
    if (dir_len_after_slash + label_len + 1 > NK_CONSOLE_FILE_PATH_MAX) {
        NK_ASSERT(0); // Path too long
        nk_console_show_message(file, "Error: File path is too long.");
        return nk_false;
    }

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

    // TODO: file: Resolve the path properly, so the paths don't recurse. For example: folder/../folder
    NK_MEMCPY(data->directory + len, label, (nk_size)(label_len + 1));
    return nk_true;
}

#ifdef NK_CONSOLE_FILE_ADD_FILES
/**
 * Click handler for the file list view. Navigates into directories or selects files.
 *
 * @internal
 */
static void nk_console_file_list_view_onclick(nk_console* list_view, void* user_data) {
    NK_UNUSED(user_data);
    nk_console* file = nk_console_file_button_get_file_widget(list_view);
    if (file == NULL || file->data == NULL) {
        return;
    }
    nk_console_file_data* data = (nk_console_file_data*)file->data;

    nk_console_list_view_data* lv_data = (nk_console_list_view_data*)list_view->data;
    nk_uint selected = lv_data->selected;
    if (selected >= cvector_size(data->entries)) {
        return;
    }

    nk_console_file_entry* entry = &data->entries[selected];
    if (!nk_console_file_append_to_directory(data, file, entry->label)) {
        return;
    }

    if (entry->is_directory) {
        // Navigate into the directory.
        nk_console_set_active_parent(file);
        nk_console_add_event(file, NK_CONSOLE_EVENT_POST_RENDER_ONCE, &nk_console_file_refresh);
    }
    else {
        // Copy the path to the file buffer.
        int desired_length = nk_strlen(data->directory);
        if (desired_length >= data->file_path_buffer_size) {
            NK_ASSERT(0); // File path is too long
            nk_console_show_message(file, "Error: File path is too long.");
        }
        else {
            NK_MEMCPY(data->file_path_buffer, data->directory, (nk_size)desired_length);
            data->file_path_buffer[desired_length] = '\0';
            nk_console_trigger_event(file, NK_CONSOLE_EVENT_CHANGED);
        }

        // Exit the file browser.
        nk_console_navigate_back(file);
    }
}
#endif

/**
 * Click handler for the "select this directory" button in directory-selection mode.
 *
 * @internal
 */
static void nk_console_file_select_dir_onclick(nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    nk_console* file = nk_console_file_button_get_file_widget(button);
    if (file == NULL || file->data == NULL) {
        return;
    }
    nk_console_file_data* data = (nk_console_file_data*)file->data;

    int desired_length = nk_strlen(data->directory);
    if (desired_length >= data->file_path_buffer_size) {
        NK_ASSERT(0); // Directory path is too long
        nk_console_show_message(file, "Error: Directory path is too long.");
    }
    else {
        NK_MEMCPY(data->file_path_buffer, data->directory, (nk_size)desired_length);
        data->file_path_buffer[desired_length] = '\0';
        nk_console_trigger_event(file, NK_CONSOLE_EVENT_CHANGED);
    }

    // Exit the file browser.
    nk_console_navigate_back(file);
}

/**
 * Event hanlder for when the user clicks on an individual file entry.
 *
 * @internal
 */
static void nk_console_file_entry_onclick(nk_console* button, void* user_data) {
    NK_UNUSED(user_data);
    if (button == NULL || button->label == NULL) {
        return;
    }

    nk_console* file = nk_console_file_button_get_file_widget(button);
    if (file == NULL || file->data == NULL) {
        return;
    }

    nk_console_file_data* data = (nk_console_file_data*)file->data;
    if (!nk_console_file_append_to_directory(data, file, button->label)) {
        return;
    }

    // Navigate into the directory (or back to parent via "..").
    nk_console_set_active_parent(file);
    nk_console_add_event(file, NK_CONSOLE_EVENT_POST_RENDER_ONCE, &nk_console_file_refresh);
}

NK_API nk_console* nk_console_file_add_entry(nk_console* parent, const char* path, nk_bool is_directory) {
    if (parent == NULL || path == NULL || path[0] == '\0') {
        return NULL;
    }

    nk_console* file = nk_console_file_button_get_file_widget(parent);
    if (file == NULL || file->data == NULL) {
        return NULL;
    }

    nk_console_file_data* data = (nk_console_file_data*)file->data;

    // Are we only selecting directories?
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

    // Copy the basename as the entry label.
    const char* basename = nk_console_file_basename(path);
    nk_size basename_len = (nk_size)nk_strlen(basename);
    char* label = (char*)NK_CONSOLE_MALLOC(nk_handle_id(0), NULL, basename_len + 1);
    if (label == NULL) {
        return NULL;
    }
    NK_MEMCPY(label, basename, basename_len);
    label[basename_len] = '\0';

    nk_console_file_entry entry;
    entry.label = label;
    entry.is_directory = is_directory;
    cvector_push_back(data->entries, entry);

    // Return the file widget as a non-NULL success indicator.
    return file;
}

/**
 * Gets the length of the directory string of the given file path.
 *
 * @internal
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
 * Fills the list view with the files in the current directory.
 */
NK_API void nk_console_file_refresh(nk_console* widget, void* user_data) {
    NK_UNUSED(user_data);
    widget = nk_console_file_button_get_file_widget(widget);
    if (widget == NULL || widget->data == NULL) {
        return;
    }

    nk_console_file_data* data = (nk_console_file_data*)widget->data;

    // Clear existing entries.
    nk_console_file_entries_clear(data);

    // Build the static children (cancel, directory label, parent dir button) on the first call only.
    // children[1]'s label points directly to data->directory, so it auto-updates on subsequent calls.
    if (widget->children == NULL || cvector_empty(widget->children)) {
        // Add the back/cancel button
        nk_console* cancelButton = nk_console_button_onclick(widget, "Cancel", &nk_console_button_back);
        nk_console_button_set_symbol(cancelButton, NK_SYMBOL_X);

        // Show the active directory.
        if (!data->select_directory) {
            // Active directory label
            nk_console* activeLabel = nk_console_label(widget, data->directory);
            activeLabel->alignment = NK_TEXT_CENTERED;
        }
        else {
            // Add a button to select the current directory.
            nk_console* button = nk_console_button(widget, data->directory);
            nk_console_button_set_symbol(button, NK_SYMBOL_CIRCLE_SOLID);
            nk_console_set_tooltip(button, "Use this directory");
            nk_console_add_event(button, NK_CONSOLE_EVENT_CLICKED, &nk_console_file_select_dir_onclick);
        }

        // Add the parent directory button
        nk_console* parent_directory_button = nk_console_button_onclick(widget, "..", &nk_console_file_entry_onclick);
        nk_console_button_set_symbol(parent_directory_button, NK_SYMBOL_TRIANGLE_LEFT);
        nk_console_set_tooltip(parent_directory_button, "Navigate to the parent directory");
    }

    // Focus the parent directory button (always children[2]).
    NK_ASSERT(cvector_size(widget->children) > 2 && widget->children[2] != NULL);
    nk_console_set_active_widget(widget->children[2]);

#ifdef NK_CONSOLE_FILE_ADD_FILES
    // Populate the entries array via the file system callback.
    NK_CONSOLE_FILE_ADD_FILES(widget, data->directory);

    // Show at least 1 item so the get_label callback can display "(Empty directory)".
    nk_uint display_count = cvector_empty(data->entries) ? 1 : (nk_uint)cvector_size(data->entries);
    if (cvector_size(widget->children) >= 4 && widget->children[3]->type == NK_CONSOLE_LIST_VIEW) {
        // Update the existing list view in place.
        nk_console_list_view_set_item_count(widget->children[3], display_count);
    }
    else {
        // Create the list view.
        nk_console* list_view = nk_console_list_view(widget, "file_entries", 10, display_count, &nk_console_file_list_view_get_label);
        nk_console_add_event(list_view, NK_CONSOLE_EVENT_CLICKED, &nk_console_file_list_view_onclick);
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
 * Event handler to clear out unneeded data for the file widget.
 *
 * @see nk_console_file_event_back
 * @internal
 */
static void nk_console_file_event_back_post_render(nk_console* file, void* user_data) {
    // Remove all the children, since we don't need them.
    nk_console_free_children(file);

    // Clear out all the file entries too.
    nk_console_file_event_destroy(file, user_data);
}

/**
 * Event handler to clear out all unneeded data when not using the widget.
 * @internal
 */
static void nk_console_file_event_back(nk_console* file, void* user_data) {
    NK_UNUSED(user_data);
    // Clear it out at post-render to avoid segfaults.
    nk_console_add_event(file, NK_CONSOLE_EVENT_POST_RENDER_ONCE, &nk_console_file_event_back_post_render);
}

/**
 * Event handler: Called when the make file button is clicked.
 *
 * Will build out the sub-elements to select a file.
 *
 * @internal
 */
static void nk_console_file_event_clicked(nk_console* button, void* user_data) {
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

    nk_console_add_event(widget, NK_CONSOLE_EVENT_CLICKED, &nk_console_file_event_clicked);
    nk_console_add_event(widget, NK_CONSOLE_EVENT_BACK, &nk_console_file_event_back);
    nk_console_add_event(widget, NK_CONSOLE_EVENT_DESTROYED, &nk_console_file_event_destroy);

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
