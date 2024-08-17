/**
 * File System for nuklear_console.
 *
 * Configuration:
 * - NK_CONSOLE_FILE_ADD_FILES: Callback which is used to add files to the file widget, matching the signature of nk_console_file_add_files_tinydir().
 * - NK_CONSOLE_ENABLE_TINYDIR: Use the tinydir library to list files in a directory. https://github.com/cxong/tinydir
 * - NK_CONSOLE_ENABLE_RAYLIB or RAYLIB_VERSION: When defined, will use raylib to enumerate the files. https://github.com/RobLoach/raylib-nuklear
 * - When no file system is enabled, clicking on the Select File button will show an error message.
 */

#ifndef NK_CONSOLE_FILE_SYSTEM_H__
#define NK_CONSOLE_FILE_SYSTEM_H__

#if defined(__cplusplus)
extern "C" {
#endif

// Nothing.

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_FILE_SYSTEM_H__

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
#endif // NK_CONSOLE_FILE_ADD_FILES_TINYDIR_H

#ifndef NK_CONSOLE_FILE_ADD_FILES_TINYDIR_SKIP

// tinydir uses the same memory function signatures as cvector.
#ifndef _TINYDIR_MALLOC
#define _TINYDIR_MALLOC cvector_clib_malloc
#endif // _TINYDIR_MALLOC
#ifndef _TINYDIR_FREE
#define _TINYDIR_FREE cvector_clib_free
#endif // _TINYDIR_FREE

#include NK_CONSOLE_FILE_ADD_FILES_TINYDIR_H
#endif // NK_CONSOLE_FILE_ADD_FILES_TINYDIR_SKIP

/**
 * Iterate through the files in the given directory, and add the contents as widgets.
 *
 * @param parent The file widget.
 * @param directory The directory to enumerate.
 *
 * @return True if there were entries added.
 *
 * @see nk_console_file_add_entry()
 */
static nk_bool nk_console_file_add_files_tinydir(nk_console* parent, const char* directory) {
    if (parent == NULL || directory == NULL) {
        return nk_false;
    }

    tinydir_dir dir;
    if (tinydir_open_sorted(&dir, directory) == -1) {
        return nk_false;
    }

    nk_bool result = nk_false;

    // Iterate through the files and add each entry.
    for (size_t i = 0; i < dir.n_files; i++) {
        tinydir_file file;
        tinydir_readfile_n(&dir, &file, i);
        if (nk_console_file_add_entry(parent, file.name, file.is_dir == 0 ? nk_false : nk_true) == nk_true) {
            result = nk_true;
        }
    }

    // Close the directory.
    tinydir_close(&dir);

    return result;
}

// Tell the file widget to use the tinydir file system.
#define NK_CONSOLE_FILE_ADD_FILES nk_console_file_add_files_tinydir

// Raylib support
#elif defined(NK_CONSOLE_ENABLE_RAYLIB) || defined(RAYLIB_VERSION)

/**
 * nuklear_console_file callback to iterate through a directory and ad all entries from the given path.
 *
 * @param console The parent files widget.
 * @param path The path to enumerate.
 *
 * @return True if there were entries that were added.
 *
 * @see nk_console_file_add_entry()
 */
static nk_bool nk_console_file_add_files_raylib(nk_console* console, const char* path) {
    FilePathList filePathList = LoadDirectoryFiles(path);

    nk_bool result = nk_false;

    // Directories
    for (int i = 0; i < filePathList.count; i++) {
        if (DirectoryExists(filePathList.paths[i])) {
            if (nk_console_file_add_entry(console, filePathList.paths[i], nk_true)) {
                result = nk_true;
            }
        }
    }

    // Files
    for (int i = 0; i < filePathList.count; i++) {
        if (FileExists(filePathList.paths[i]) && !DirectoryExists(filePathList.paths[i])) {
            if (nk_console_file_add_entry(console, filePathList.paths[i], nk_false)) {
                result = nk_true;
            }
        }
    }

    UnloadDirectoryFiles(filePathList);

    return result;
}

// Tell the file widget to use the raylib file system.
#define NK_CONSOLE_FILE_ADD_FILES nk_console_file_add_files_raylib

#else // !NK_CONSOLE_ENABLE_TINYDIR && !NK_CONSOLE_ENABLE_RAYLIB

/**
 * Since there is no file system found, clicking Select File buttons will report an error message.
 *
 * @param parent The file widget.
 * @param directory The directory to enumerate.
 *
 * @see nk_console_file_add_entry()
 */
static nk_bool nk_console_file_add_files_diabled(nk_console* file, const char* directory) {
    NK_UNUSED(directory);
    if (file == NULL) {
        return nk_false;
    }

    // Requires NK_CONSOLE_ENABLE_TINYDIR or another file system library.
    nk_console_show_message(file, "Error: File system not available.");

    // Go back to the parent widget, and disable the widget.
    if (file->parent != NULL) {
        file->disabled = nk_true;
        nk_console_set_active_parent(file->parent);
    }

    // The file system is disabled, not erroring out.
    return nk_true;
}

// Tell the file widget that the file system is disabled.
#define NK_CONSOLE_FILE_ADD_FILES nk_console_file_add_files_diabled

#endif // NK_CONSOLE_ENABLE_TINYDIR
#endif // NK_CONSOLE_FILE_ADD_FILES

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_FILE_SYSTEM_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
