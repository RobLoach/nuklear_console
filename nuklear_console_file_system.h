#ifndef NK_CONSOLE_FILE_SYSTEM_H__
#define NK_CONSOLE_FILE_SYSTEM_H__

#if defined(__cplusplus)
extern "C" {
#endif


#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_FILE_SYSTEM_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_FILE_SYSTEM_IMPLEMENTATION_ONCE
#define NK_CONSOLE_FILE_SYSTEM_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

// TODO: Allow disabling tinydir
#ifndef NK_CONSOLE_FILE_ADD_FILES
#ifdef NK_CONSOLE_ENABLE_TINYDIR

#ifndef NK_CONSOLE_FILE_ADD_FILES_TINYDIR_H
#define NK_CONSOLE_FILE_ADD_FILES_TINYDIR_H "vendor/tinydir/tinydir.h"
#endif  // NK_CONSOLE_FILE_ADD_FILES_TINYDIR_H
#ifndef NK_CONSOLE_FILE_ADD_FILES_TINYDIR_SKIP

// tinydir uses the same memory function signatures as cvector.
#ifndef _TINYDIR_MALLOC
#define _TINYDIR_MALLOC cvector_clib_malloc
#endif  // _TINYDIR_MALLOC
#ifndef _TINYDIR_FREE
#define _TINYDIR_FREE cvector_clib_free
#endif  // _TINYDIR_FREE

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

// Raylib support
#elif defined(RAYLIB_VERSION)

/**
 * Destroy callback; Clear the raylib file system memory.
 */
void nk_console_file_destroy_raylib(nk_console* console) {
    FilePathList* list = (FilePathList*)nk_console_file_get_file_user_data(console);
    if (list != NULL) {
        // Clear the directory entries.
        UnloadDirectoryFiles(*list);

        // Clear our the FilePathList data.
        MemFree(list);
    }
}

/**
 * nuklear_console_file callback to iterate add an entry for each file in the given path.
 */
void nk_console_file_add_files_raylib(nk_console* console, const char* path) {
    FilePathList* list = nk_console_file_get_file_user_data(console);
    if (list == NULL) {
        list = (FilePathList*)MemAlloc(sizeof(FilePathList));
        nk_console_file_set_file_user_data(console, (void*)list);
        console->destroy = nk_console_file_destroy_raylib;
    }
    else {
        UnloadDirectoryFiles(*list);
    }

    TraceLog(LOG_INFO, "Loading files from directory: %s", path);
    FilePathList filePathList = LoadDirectoryFiles(path);
    for (int i = 0; i < filePathList.count; i++) {
        TraceLog(LOG_INFO, "  %s", filePathList.paths[i]);
        nk_console_file_add_entry(console, filePathList.paths[i], DirectoryExists(filePathList.paths[i]));
    }
    *list = filePathList;
}

#define NK_CONSOLE_FILE_ADD_FILES nk_console_file_add_files_raylib

#else  // !NK_CONSOLE_ENABLE_TINYDIR && !RAYLIB_VERSION

/**
 * Since there is no file system, clicking on Select File buttons will show an error message.
 */
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

#if defined(__cplusplus)
}
#endif

#endif  // NK_CONSOLE_FILE_SYSTEM_IMPLEMENTATION_ONCE
#endif  // NK_CONSOLE_IMPLEMENTATION
