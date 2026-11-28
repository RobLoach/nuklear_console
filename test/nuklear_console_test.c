// This test suite uses pntr_nuklear to render the Nuklear Context to an image.
// https://github.com/RobLoach/pntr_nuklear

#include <assert.h>
#include <stdio.h>

#define PNTR_ENABLE_DEFAULT_FONT
#define PNTR_IMPLEMENTATION
#include "pntr.h"

// Nuklear's text editing moves overlapping memory with NK_MEMCPY, which pntr_nuklear maps to memcpy.
#include <string.h>
#define NK_MEMCPY memmove

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define PNTR_NUKLEAR_IMPLEMENTATION
#define NK_BUTTON_TRIGGER_ON_RELEASE
#include "pntr_nuklear.h"

#ifndef NK_CONSOLE_NO_GAMEPAD
#define NK_GAMEPAD_NONE
#define NK_GAMEPAD_IMPLEMENTATION
#include "nuklear_gamepad.h"
#endif

#define NK_CONSOLE_IMPLEMENTATION
#include "nuklear_console.h"

static void test_event_counter(nk_console* widget, void* user_data) {
    (void)widget;
    if (user_data != NULL) {
        (*(int*)user_data)++;
    }
}

typedef struct test_post_render_once_data {
    nk_console* widget;
    int callback_count;
    int destructor_count;
} test_post_render_once_data;

static void test_post_render_once_callback(nk_console* widget, void* user_data) {
    test_post_render_once_data* data = (test_post_render_once_data*)user_data;
    assert(data != NULL);
    assert(widget == data->widget);
    data->callback_count++;
}

static void test_post_render_once_destructor(nk_console* widget, void* user_data) {
    test_post_render_once_data* data = (test_post_render_once_data*)user_data;
    assert(data != NULL);
    assert(widget == data->widget);
    data->destructor_count++;
}

static const char* list_view_get_label(struct nk_console* list_view, nk_uint index) {
    (void)list_view;
    static const char* items[] = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};
    if (index < 5) {
        return items[index];
    }
    return NULL;
}

/**
 * End a frame without drawing it. Building the commands resets the popup state that only nk_build() clears,
 * so a stale tooltip from this window can't loop the next drawn frame's commands (#342).
 */
static void test_end_frame(struct nk_context* ctx) {
    nk__begin(ctx);
    nk_clear(ctx);
}

/**
 * Render one frame of the given console, typing the UTF-8 text and pressing the key.
 */
static void test_render_frame(nk_console* console, const char* text, enum nk_keys key) {
    struct nk_context* ctx = console->ctx;
    nk_input_begin(ctx);
    if (text != NULL) {
        int len = (int)strlen(text);
        int i = 0;
        while (i < len) {
            nk_rune unicode = 0;
            int glyph_len = nk_utf_decode(text + i, &unicode, len - i);
            assert(glyph_len > 0);
            nk_input_unicode(ctx, unicode);
            i += glyph_len;
        }
    }
    if (key != NK_KEY_NONE) {
        nk_input_key(ctx, key, nk_true);
    }
    nk_input_end(ctx);
    nk_console_render_window(console, "masked_test", nk_rect(0, 0, 300, 800), NK_WINDOW_TITLE);
    test_end_frame(ctx);

    // Release the key so the next frame doesn't see it pressed.
    if (key != NK_KEY_NONE) {
        nk_input_begin(ctx);
        nk_input_key(ctx, key, nk_false);
        nk_input_end(ctx);
    }
}

/**
 * Render one frame of the given console with the key tapped, optionally while holding shift.
 */
static void test_tap_key(nk_console* console, enum nk_keys key, nk_bool shift) {
    struct nk_context* ctx = console->ctx;
    nk_input_begin(ctx);
    nk_input_key(ctx, NK_KEY_SHIFT, shift);
    nk_input_key(ctx, key, nk_true);
    nk_input_key(ctx, key, nk_false);
    nk_input_end(ctx);
    nk_console_render_window(console, "tap_test", nk_rect(0, 0, 300, 800), NK_WINDOW_TITLE);
    test_end_frame(ctx);
    nk_input_begin(ctx);
    nk_input_key(ctx, NK_KEY_SHIFT, nk_false);
    nk_input_end(ctx);
}

int main() {
    // nk_console_file_normalize_path()
    {
        char path[256];

        // "." segments are removed.
        snprintf(path, sizeof(path), "./a");
        nk_console_file_normalize_path(path, sizeof(path));
        assert(strcmp(path, "a") == 0);

        // ".." collapses the preceding segment.
        snprintf(path, sizeof(path), "a/b/../c");
        nk_console_file_normalize_path(path, sizeof(path));
        assert(strcmp(path, "a/c") == 0);

        // Leading ".." in a relative path is preserved.
        snprintf(path, sizeof(path), "../a");
        nk_console_file_normalize_path(path, sizeof(path));
        assert(strcmp(path, "../a") == 0);

        // Multiple ".." that cancel normal segments.
        snprintf(path, sizeof(path), "a/b/../../c");
        nk_console_file_normalize_path(path, sizeof(path));
        assert(strcmp(path, "c") == 0);

        // Consecutive separators are collapsed.
        snprintf(path, sizeof(path), "a//b");
        nk_console_file_normalize_path(path, sizeof(path));
        assert(strcmp(path, "a/b") == 0);

        // Backslash treated as separator.
        snprintf(path, sizeof(path), "a\\b\\..\\c");
        nk_console_file_normalize_path(path, sizeof(path));
        assert(strcmp(path, "a/c") == 0);

        // Mixed separators are normalized to forward slash.
        snprintf(path, sizeof(path), "a\\b/c\\d");
        nk_console_file_normalize_path(path, sizeof(path));
        assert(strcmp(path, "a/b/c/d") == 0);
    }

    // Load Nuklear
    pntr_font* font = pntr_load_font_default();
    assert(font != NULL);
    struct nk_context* ctx = pntr_load_nuklear(font);
    assert(ctx != NULL);

    // nk_console_init()
    struct nk_console* console = nk_console_init(ctx);
    assert(console != NULL);

    // Gamepad
    #ifndef NK_CONSOLE_NO_GAMEPAD
    struct nk_gamepads gamepads;
    assert(nk_gamepad_init(&gamepads, ctx, NULL) == nk_true);
    nk_console_set_gamepads(console, &gamepads);
    #endif

    // nk_console_label()
    {
        nk_console* label = nk_console_label(console, "Simple label.");
        assert(label != NULL);
    }

    // nk_console_button()
    {
        nk_console* button = nk_console_button(console, "Button");
        assert(button != NULL);

        // nk_image_id() must set has_image so the render path draws it.
        nk_console_button_set_image(button, nk_image_id(1));
        nk_console_button_data* btn_data = (nk_console_button_data*)button->data;
        assert(btn_data->has_image == nk_true);
        struct nk_image btn_img = nk_console_button_get_image(button);
        assert(btn_img.handle.id == 1);
        // Reset so the render pass doesn't try to draw an invalid texture.
        btn_data->has_image = nk_false;
    }

    // nk_console_checkbox()
    {
        static nk_bool checkbox_value = nk_true;
        nk_console* checkbox = nk_console_checkbox(console, "Checkbox", &checkbox_value);
        assert(checkbox != NULL);
    }

    // nk_console_spacing()
    {
        nk_console_label(console, "Spacing:");
        nk_console* spacing = nk_console_spacing(console, 3);
        assert(spacing != NULL);
    }

    // nk_console_progress()
    {
        static nk_size process_value = 20;
        nk_console* progress = nk_console_progress(console, "Progress Bar", &process_value, 100);
        assert(progress != NULL);
    }

    // nk_console_input_gamepad()
    {
        static int gamepad_number = 0;
        static enum nk_gamepad_button gamepad_button = NK_GAMEPAD_BUTTON_LB;
        nk_console* input = nk_console_input_gamepad(console, "Input Button", -1, &gamepad_number, &gamepad_button);
        assert(input != NULL);
        assert(nk_console_input_is_gamepad(input) == nk_true);
        assert(nk_console_input_is_key(input) == nk_false);
        assert(nk_console_input_is_mouse(input) == nk_false);

        // A keyboard key (typed character or special key) is stored in an
        // nk_rune as an NK_CONSOLE_KEY_* value.
        static nk_rune out_key = NK_CONSOLE_KEY_ENTER;
        nk_console* input_key = nk_console_input_key(console, "Key Input", &out_key);
        assert(input_key != NULL);
        assert(nk_console_input_is_key(input_key) == nk_true);

        // A mouse button binding.
        static enum nk_buttons out_mouse = NK_BUTTON_LEFT;
        nk_console* input_mouse = nk_console_input_mouse(console, "Mouse Input", &out_mouse);
        assert(input_mouse != NULL);
        assert(nk_console_input_is_mouse(input_mouse) == nk_true);

        // Value getters return the captured value of the active source, and the
        // sentinel for sources that aren't active on that widget.
        assert(nk_console_input_get_gamepad(input) == NK_GAMEPAD_BUTTON_LB);
        assert(nk_console_input_get_key(input_key) == NK_CONSOLE_KEY_ENTER);
        assert(nk_console_input_get_mouse(input_mouse) == NK_BUTTON_LEFT);
        assert(nk_console_input_get_key(input) == NK_CONSOLE_KEY_NONE);
        assert(nk_console_input_get_gamepad(input_key) == NK_GAMEPAD_BUTTON_INVALID);
        assert((int)nk_console_input_get_mouse(input_key) == -1);

        // Resolvers: special keys and typed characters share one NK_CONSOLE_KEY_*
        // space. Control keys map to ASCII; printable chars to their codepoint.
        assert(nk_console_input_rune_from_keys(NK_KEY_ENTER) == NK_CONSOLE_KEY_ENTER);
        assert(nk_console_input_rune_from_keys(NK_KEY_TAB) == NK_CONSOLE_KEY_TAB);
        assert(nk_console_input_rune_from_keys(NK_KEY_UP) == NK_CONSOLE_KEY_UP);
        assert(nk_console_input_rune_from_keys(NK_KEY_NONE) == NK_CONSOLE_KEY_NONE);

        // Port back to enum nk_keys; printable characters have no equivalent.
        assert(nk_console_input_rune_to_keys(NK_CONSOLE_KEY_ENTER) == NK_KEY_ENTER);
        assert(nk_console_input_rune_to_keys(NK_CONSOLE_KEY_UP) == NK_KEY_UP);
        assert(nk_console_input_rune_to_keys(NK_CONSOLE_KEY_DELETE) == NK_KEY_DEL);
        assert(nk_console_input_rune_to_keys((nk_rune)'A') == NK_KEY_NONE);

        // Names cover special keys, control keys, and printable codepoints
        // (incl. ',' == 44, which collided under the old nk_rune scheme).
        assert(strcmp(nk_console_input_key_name(NK_CONSOLE_KEY_ENTER), "Enter") == 0);
        assert(strcmp(nk_console_input_key_name(NK_CONSOLE_KEY_NONE), "<None>") == 0);
        assert(strcmp(nk_console_input_key_name(NK_CONSOLE_KEY_UP), "Up") == 0);
        assert(strcmp(nk_console_input_key_name((nk_rune)'A'), "A") == 0);
        assert(strcmp(nk_console_input_key_name((nk_rune)' '), "Space") == 0);
        assert(strcmp(nk_console_input_key_name((nk_rune)','), ",") == 0);

        // Defaults: getters return initial values before any set.
        assert(nk_console_input_get_gamepad_default(input) == NK_GAMEPAD_BUTTON_INVALID);
        assert(nk_console_input_get_key_default(input_key) == NK_CONSOLE_KEY_NONE);
        assert((int)nk_console_input_get_mouse_default(input_mouse) == -1);

        // Defaults: round-trip set then get.
        nk_console_input_set_gamepad_default(input, NK_GAMEPAD_BUTTON_A);
        assert(nk_console_input_get_gamepad_default(input) == NK_GAMEPAD_BUTTON_A);

        nk_console_input_set_key_default(input_key, NK_CONSOLE_KEY_ENTER);
        assert(nk_console_input_get_key_default(input_key) == NK_CONSOLE_KEY_ENTER);

        nk_console_input_set_mouse_default(input_mouse, NK_BUTTON_RIGHT);
        assert(nk_console_input_get_mouse_default(input_mouse) == NK_BUTTON_RIGHT);
    }

    // nk_console_input() combinations
    {
        // Before any capture, is_*() reflects which pointers are configured.
        // (Statics so the stored pointers stay valid when the console renders.)
        static int combo_gp_number = 0;
        static enum nk_gamepad_button combo_gp_button = NK_GAMEPAD_BUTTON_A;
        static nk_rune combo_key = NK_CONSOLE_KEY_ENTER;
        static enum nk_buttons combo_mouse = NK_BUTTON_LEFT;
        nk_console* combo = nk_console_input(console, "Combination", -1, &combo_gp_number, &combo_gp_button, &combo_key, &combo_mouse);
        assert(nk_console_input_is_gamepad(combo) == nk_true);
        assert(nk_console_input_is_key(combo) == nk_true);
        assert(nk_console_input_is_mouse(combo) == nk_true);

        // Gamepad-or-key: both pointers configured, both return true before capture.
        static int gpkey_number = 0;
        static enum nk_gamepad_button gpkey_button = NK_GAMEPAD_BUTTON_A;
        static nk_rune gpkey_key = NK_CONSOLE_KEY_ENTER;
        nk_console* gpkey = nk_console_input(console, "Gamepad or Key", -1, &gpkey_number, &gpkey_button, &gpkey_key, NULL);
        assert(nk_console_input_is_key(gpkey) == nk_true);
        assert(nk_console_input_is_gamepad(gpkey) == nk_true);
        assert(nk_console_input_is_mouse(gpkey) == nk_false);
    }

    // nk_console_combobox()
    {
        static int value_combobox = 3;
        nk_console* combobox = nk_console_combobox(console, "ComboBox", "Fists;Chainsaw;Pistol;Shotgun;Chaingun", ';', &value_combobox);
        assert(combobox != NULL);
    }

    // nk_console_combobox() with NULL items must not crash
    {
        static int value_null_combobox = 5;
        nk_console* combobox_null = nk_console_combobox(console, "NullComboBox", NULL, '|', &value_null_combobox);
        assert(combobox_null != NULL);
        assert(value_null_combobox == 0);
        // Only the back button should be a child (no item children).
        assert(cvector_size(combobox_null->children) == 1);
    }

    // nk_console_property_int/float()
    {
        static int property_int_test = 10;
        static float property_float_test = 0.5f;
        nk_console* property_int = nk_console_property_int(console, "Property Int", 10, &property_int_test, 30, 1, 1);
        nk_console* property_float = nk_console_property_float(console, "Property Float", 0.0f, &property_float_test, 2.0f, 0.1f, 1);
        assert(property_int != NULL);
        assert(property_float != NULL);
    }

    // nk_console_slider_float/int()
    {
        static float slider_float_test = 1.0f;
        static int slider_int_test = 15;
        nk_console* slider_float = nk_console_slider_float(console, "Slider Float", 0.0f, &slider_float_test, 2.0f, 0.1f);
        nk_console* slider_int = nk_console_slider_int(console, "Slider Int", 0, &slider_int_test, 20, 1);
        assert(slider_float != NULL);
        assert(slider_int != NULL);
    }

    // nk_console_textedit()
    {
        static const int textedit_buffer_size = 256;
        static char textedit_buffer[256] = "brianwatling";
        nk_console* textedit = nk_console_textedit(console, "Textedit", textedit_buffer, textedit_buffer_size);
        assert(textedit != NULL);

        // The on-screen keyboard defaults to QWERTY, unless NK_CONSOLE_TEXTEDIT_KEYBOARD_ALPHABETICAL is defined.
        const nk_console_textedit_key** layout = nk_console_textedit_layout_ascii;
#ifdef NK_CONSOLE_TEXTEDIT_KEYBOARD_ALPHABETICAL
        assert(strcmp(layout[1][0].normal, "a") == 0);
        assert(strcmp(layout[1][0].shifted, "A") == 0);
#else
        assert(strcmp(layout[1][0].normal, "q") == 0);
        assert(strcmp(layout[1][0].shifted, "Q") == 0);
#endif

        // Every row, and the layout itself, is NULL-terminated like the keyboard build loop expects.
        int layout_rows = 0;
        while (layout[layout_rows] != NULL) {
            int layout_keys = 0;
            while (layout[layout_rows][layout_keys].normal != NULL) {
                layout_keys++;
            }
            assert(layout_keys > 0);
            layout_rows++;
        }
        assert(layout_rows == 4);
    }

    // nk_console_color()
    {
        static struct nk_colorf color_value = {0.31f, 1.0f, 0.48f, 1.0f};
        nk_console* color = nk_console_color(console, "Color", &color_value, NK_RGBA);
        assert(color != NULL);
    }

    // nk_console_file()
    {
        static const int file_path_buffer_size = 256;
        static char file_path_buffer[256] = "";
        nk_console* file = nk_console_file(console, "File", file_path_buffer, file_path_buffer_size);
        assert(file != NULL);

        // Backing out of the file browser keeps the widget's extension filter (regression for the
        // filter being freed along with the browser's entries).
        nk_console_file_set_filter(file, ".txt;.png");
        nk_console_trigger_event(file, NK_CONSOLE_EVENT_BACK);
        nk_console_render_window(console, "file_back_test", nk_rect(0, 0, 300, 800), NK_WINDOW_TITLE);
        assert(nk_console_file_get_filter(file) != NULL);
        assert(strcmp(nk_console_file_get_filter(file), ".txt;.png") == 0);
    }

    // nk_console_file() with paths longer than NK_CONSOLE_FILE_PATH_MAX
    {
        nk_console_top_data* top_data = (nk_console_top_data*)(nk_console_get_top(console)->data);
        cvector_clear(top_data->messages);

        // Opening the browser with an over-long selected file falls back to "." with an error message.
        static char long_file_path[NK_CONSOLE_FILE_PATH_MAX * 2 + 16];
        memset(long_file_path, 'a', sizeof(long_file_path) - 1);
        long_file_path[NK_CONSOLE_FILE_PATH_MAX + 4] = '/';
        long_file_path[sizeof(long_file_path) - 1] = '\0';
        nk_console* file = nk_console_file(console, "Long File", long_file_path, (int)sizeof(long_file_path));
        nk_console_trigger_event(file, NK_CONSOLE_EVENT_CLICKED);
        nk_console_file_data* file_data = (nk_console_file_data*)file->data;
        assert(strcmp(file_data->directory, ".") == 0);
        assert(cvector_size(top_data->messages) == 1);
        assert(strcmp(top_data->messages[0].text, "Error: File path is too long.") == 0);
        nk_console_trigger_event(file, NK_CONSOLE_EVENT_BACK);
        nk_console_render_window(console, "file_long_path_test_1", nk_rect(0, 0, 300, 800), NK_WINDOW_TITLE);
        cvector_clear(top_data->messages);

        // Navigating to ".." from a directory at the length limit shows an error rather than aborting.
        static char long_dir[NK_CONSOLE_FILE_PATH_MAX - 1];
        memset(long_dir, 'b', sizeof(long_dir) - 1);
        long_dir[sizeof(long_dir) - 1] = '\0';
        nk_console_file_set_directory(file, long_dir);
        nk_console_trigger_event(file, NK_CONSOLE_EVENT_CLICKED);
        nk_console_render_window(console, "file_long_path_test_2", nk_rect(0, 0, 300, 800), NK_WINDOW_TITLE);
        assert(cvector_size(file->children) > 2);
        nk_console_trigger_event(file->children[2], NK_CONSOLE_EVENT_CLICKED); // ".."
        assert(strcmp(file_data->directory, long_dir) == 0);
        assert(cvector_size(top_data->messages) > 0);
        assert(strcmp(top_data->messages[cvector_size(top_data->messages) - 1].text, "Error: File path is too long.") == 0);
        nk_console_trigger_event(file, NK_CONSOLE_EVENT_BACK);
        nk_console_render_window(console, "file_long_path_test_3", nk_rect(0, 0, 300, 800), NK_WINDOW_TITLE);
        cvector_clear(top_data->messages);
    }

    // nk_console_dir()
    {
        static const int dir_buffer_size = 256;
        static char dir_buffer[256] = "";
        nk_console* dir = nk_console_dir(console, "Directory", dir_buffer, dir_buffer_size);
        assert(dir != NULL);

        // Verify that setting a path with "." segments is normalized correctly.
        snprintf(dir_buffer, (size_t)dir_buffer_size, "resources/./subdir");
        nk_console_file_normalize_path(dir_buffer, dir_buffer_size);
        assert(strcmp(dir_buffer, "resources/subdir") == 0);

        // Verify that ".." collapses the preceding segment (regression for #188).
        snprintf(dir_buffer, (size_t)dir_buffer_size, "resources/subdir/../other");
        nk_console_file_normalize_path(dir_buffer, dir_buffer_size);
        assert(strcmp(dir_buffer, "resources/other") == 0);
    }

    // nk_console_dir_action()
    {
        static const int dir_action_buffer_size = 256;
        static char dir_action_buffer[256] = "";
        nk_console* dir_action = nk_console_dir_action(console, "Select Directory", dir_action_buffer, dir_action_buffer_size);
        assert(dir_action != NULL);
    }

    // Navigation, with Events
    if (nk_begin(ctx, "nav_test", nk_rect(0, 0, 300, 600), 0)) {
        nk_console* nav = nk_console_init(ctx);
        assert(nav != NULL);

        // Build tree
        nk_console* submenu = nk_console_button(nav, "Submenu");
        nk_console* leaf_a = nk_console_button(submenu, "Leaf A");
        nk_console* leaf_b = nk_console_button(submenu, "Leaf B");
        assert(submenu != NULL);
        assert(leaf_a != NULL);
        assert(leaf_b != NULL);

        // nk_console_find_by_path()
        assert(nk_console_find_by_path(nav, "Submenu") == submenu);
        assert(nk_console_find_by_path(nav, "Submenu/Leaf A") == leaf_a);
        assert(nk_console_find_by_path(nav, "Submenu/Leaf B") == leaf_b);
        assert(nk_console_find_by_path(nav, "Missing") == NULL);

        // Active parent starts at root
        assert(nk_console_active_parent(nav) == nav);

        // Navigate into submenu: target has children → active parent = submenu
        assert(nk_console_navigate_to_path(nav, "Submenu") == nk_true);
        assert(nk_console_active_parent(nav) == submenu);

        // Navigate to leaf: no children → active parent = submenu, active widget = leaf_a
        assert(nk_console_navigate_to_path(nav, "Submenu/Leaf A") == nk_true);
        assert(nk_console_active_parent(nav) == submenu);
        assert(nk_console_get_active_widget(leaf_a) == leaf_a);

        // FOCUS / BLUR fire when the active widget changes
        int focus_a = 0, blur_a = 0, focus_b = 0;
        nk_console_add_event_handler(leaf_a, NK_CONSOLE_EVENT_FOCUS, test_event_counter, &focus_a, NULL);
        nk_console_add_event_handler(leaf_a, NK_CONSOLE_EVENT_BLUR,  test_event_counter, &blur_a,  NULL);
        nk_console_add_event_handler(leaf_b, NK_CONSOLE_EVENT_FOCUS, test_event_counter, &focus_b, NULL);

        nk_console_set_active_widget(leaf_b);
        assert(blur_a  == 1);
        assert(focus_b == 1);
        assert(nk_console_get_active_widget(leaf_b) == leaf_b);

        nk_console_set_active_widget(leaf_a);
        assert(focus_a == 1);
        assert(nk_console_get_active_widget(leaf_a) == leaf_a);

        // BACK event fires on the leaving parent when navigating back
        int back_count = 0;
        nk_console_add_event_handler(submenu, NK_CONSOLE_EVENT_BACK, test_event_counter, &back_count, NULL);

        nk_console_button_back(leaf_a, NULL);
        assert(back_count == 1);
        assert(nk_console_active_parent(nav) == nav);

        // Navigate back from a widget inside a row skips the row and goes to
        // the row's parent, preventing a crash (#227).
        {
            nk_console* menu = nk_console_button(nav, "Menu");
            nk_console* row = nk_console_row_begin(menu);
            nk_console* btn_in_row = nk_console_button(row, "Row Button");
            nk_console_row_end(row);
            assert(btn_in_row->parent == row);
            assert(row->type == NK_CONSOLE_ROW);

            // Navigate into menu so it becomes the active parent.
            nk_console_set_active_parent(menu);
            assert(nk_console_active_parent(nav) == menu);

            // Simulate navigating back when the active widget is inside a row.
            // nk_console_navigate_back(btn_in_row->parent) == navigate_back(row).
            // The row should be skipped; active parent should become nav (row->parent->parent).
            nk_console_navigate_back(row);
            assert(nk_console_active_parent(nav) == nav);
        }

        // Navigate back from a button *inside* a row that has its own children:
        // opening the button then pressing Back must land on the row's parent
        // page (not the row itself, which can't be an active parent) (#227).
        {
            nk_console* page = nk_console_button(nav, "Row Page");
            nk_console* row = nk_console_row_begin(page);
            nk_console* tools = nk_console_button(row, "Tools");
            nk_console_button(tools, "Hammer"); // gives tools children, making it a sub-page
            nk_console_row_end(row);
            assert(tools->parent == row);

            // Open the tools sub-page.
            nk_console_set_active_parent(tools);
            assert(nk_console_active_parent(nav) == tools);

            // Press Back from inside tools: navigate_back(back_button->parent) == navigate_back(tools).
            nk_console_navigate_back(tools);

            // The row is skipped: the active parent is the row's parent page, and
            // focus lands on the row (so its active child stays highlighted).
            assert(nk_console_active_parent(nav) == page);
            assert(nk_console_get_active_widget(row) == row);
        }

        nk_console_free(nav);
    }
    nk_end(ctx);

    // Navigation outside a frame: nk_console_set_active_parent() skips the
    // window-scroll reset when no window is current, so navigating between
    // frames doesn't assert inside nk_window_set_scroll().
    {
        nk_console* nav = nk_console_init(ctx);
        assert(nav != NULL);
        nk_console* submenu = nk_console_button(nav, "Submenu");
        nk_console* leaf = nk_console_button(submenu, "Leaf");
        assert(leaf != NULL);

        assert(nk_console_navigate_to_path(nav, "Submenu") == nk_true);
        assert(nk_console_active_parent(nav) == submenu);
        assert(nk_console_navigate_to_path(nav, "Submenu/Leaf") == nk_true);
        assert(nk_console_get_active_widget(leaf) == leaf);

        nk_console_free(nav);
    }

    // Scroll on parent switch (#265): switching to a parent with an active
    // widget queues it as the render loop's scroll target instead of resetting
    // the scroll to the top.
    {
        nk_console* nav = nk_console_init(ctx);
        assert(nav != NULL);
        nk_console* submenu = nk_console_button(nav, "Submenu");
        nk_console* leaf_a = nk_console_button(submenu, "Leaf A");
        nk_console* leaf_b = nk_console_button(submenu, "Leaf B");
        assert(leaf_a != NULL && leaf_b != NULL);
        nk_console_top_data* top_data = (nk_console_top_data*)nav->data;

        // Without an active widget, no scroll target is queued.
        nk_console_set_active_parent(submenu);
        assert(top_data->scroll_to_widget == NULL);

        // Navigating to a leaf queues the leaf itself, not the previously active widget.
        nk_console_set_active_widget(leaf_a);
        assert(nk_console_navigate_to_path(nav, "Submenu/Leaf B") == nk_true);
        assert(top_data->scroll_to_widget == leaf_b);

        // Re-entering a parent queues its remembered active widget.
        nk_console_set_active_parent(nav);
        top_data->scroll_to_widget = NULL;
        nk_console_set_active_parent(submenu);
        assert(top_data->scroll_to_widget == leaf_b);

        // Freeing the queued widget clears the scroll target rather than leaving it dangling.
        nk_console_set_active_widget(leaf_a);
        submenu->children[1] = NULL; // Detach leaf_b, like a listing rebuild would.
        nk_console_free(leaf_b);
        assert(top_data->scroll_to_widget == NULL);

        nk_console_free(nav);
    }

    // nk_console_free() on a child leaves no dangling pointers (#86).
    {
        nk_console* nav = nk_console_init(ctx);
        nk_console* submenu = nk_console_button(nav, "Submenu");
        nk_console* leaf_a = nk_console_button(submenu, "Leaf A");
        nk_console* leaf_b = nk_console_button(submenu, "Leaf B");
        nk_console* leaf_c = nk_console_button(submenu, "Leaf C");
        nk_console* other = nk_console_button(nav, "Other");
        nk_console_set_tooltip(leaf_b, "Tooltip");
        nk_console_top_data* top_data = (nk_console_top_data*)nav->data;

        // Freeing the active widget removes it and focuses the sibling that took its place.
        assert(nk_console_navigate_to_path(nav, "Submenu/Leaf B") == nk_true);
        top_data->tooltip_last = leaf_b->tooltip;
        nk_console_free(leaf_b);
        assert(cvector_size(submenu->children) == 2);
        assert(submenu->children[0] == leaf_a && submenu->children[1] == leaf_c);
        assert(submenu->activeWidget == leaf_c);
        assert(top_data->scroll_to_widget == NULL);
        assert(top_data->tooltip_last == NULL);

        // Freeing the last widget falls back to the previous sibling.
        nk_console_free(leaf_c);
        assert(submenu->activeWidget == leaf_a);

        // Render and navigate after freeing a child.
        nk_console_render_window(nav, "free_child_test_1", nk_rect(0, 0, 300, 400), NK_WINDOW_TITLE);
        nk_clear(ctx);
        nk_input_begin(ctx);
        nk_input_key(ctx, NK_KEY_DOWN, nk_true);
        nk_input_key(ctx, NK_KEY_DOWN, nk_false);
        nk_input_end(ctx);
        nk_console_render_window(nav, "free_child_test_1", nk_rect(0, 0, 300, 400), NK_WINDOW_TITLE);
        nk_clear(ctx);
        nk_input_begin(ctx);
        nk_input_end(ctx);
        assert(nk_console_active_parent(nav) == submenu);
        assert(submenu->activeWidget == leaf_a);

        // Freeing the active parent switches back to its parent.
        nk_console_set_active_widget(submenu);
        nk_console_free(submenu);
        assert(nk_console_active_parent(nav) == nav);
        assert(cvector_size(nav->children) == 1);
        assert(nav->activeWidget == other);
        nk_console_render_window(nav, "free_child_test_2", nk_rect(0, 0, 300, 400), NK_WINDOW_TITLE);
        nk_clear(ctx);
        nk_input_begin(ctx);
        nk_input_key(ctx, NK_KEY_UP, nk_true);
        nk_input_key(ctx, NK_KEY_UP, nk_false);
        nk_input_end(ctx);
        nk_console_render_window(nav, "free_child_test_2", nk_rect(0, 0, 300, 400), NK_WINDOW_TITLE);
        nk_clear(ctx);
        nk_input_begin(ctx);
        nk_input_end(ctx);
        assert(nav->activeWidget == other);

        // Freeing an ancestor of the active parent switches to the ancestor's parent.
        nk_console* outer = nk_console_button(nav, "Outer");
        nk_console* inner = nk_console_button(outer, "Inner");
        nk_console_button(inner, "Deep");
        assert(nk_console_navigate_to_path(nav, "Outer/Inner/Deep") == nk_true);
        assert(nk_console_active_parent(nav) == inner);
        nk_console_free(outer);
        assert(nk_console_active_parent(nav) == nav);
        assert(cvector_size(nav->children) == 1);
        nk_console_render_window(nav, "free_child_test_3", nk_rect(0, 0, 300, 400), NK_WINDOW_TITLE);
        nk_clear(ctx);

        // Freeing a widget within a row keeps the row's active child index in range.
        nk_console* row = nk_console_row_begin(nav);
        nk_console* row_a = nk_console_button(row, "A");
        nk_console* row_b = nk_console_button(row, "B");
        nk_console* row_c = nk_console_button(row, "C");
        nk_console_row_end(row);
        ((nk_console_row_data*)row->data)->activeChild = 2;
        nk_console_free(row_a);
        assert(((nk_console_row_data*)row->data)->activeChild == 1);
        assert(row->children[1] == row_c);
        (void)row_b;

        // Freeing a tree also frees and detaches the children it references.
        size_t before_tree = cvector_size(nav->children);
        nk_console* tree = nk_console_tree(nav, "Tree", nk_true);
        nk_console_label(tree, "Tree Child A");
        nk_console_label(tree, "Tree Child B");
        assert(cvector_size(nav->children) == before_tree + 3);
        nk_console_free(tree);
        assert(cvector_size(nav->children) == before_tree);
        nk_console_render_window(nav, "free_child_test_4", nk_rect(0, 0, 300, 400), NK_WINDOW_TITLE);
        nk_clear(ctx);

        nk_console_free(nav);
    }

    // nk_console_image()
    pntr_image* image_value = pntr_load_image("resources/image.png");
    assert(image_value != NULL);
    {
        nk_console* image = nk_console_image(console, pntr_image_nk(image_value));
        nk_console_set_height(image, image_value->height);
        assert(image != NULL);
    }

    // nk_console_show_message()
    {
        nk_console_show_message(console, "This is an info message");

        // Verify that messages longer than NK_CONSOLE_MESSAGE_MAX_LENGTH are truncated.
        char long_msg[NK_CONSOLE_MESSAGE_MAX_LENGTH + 46];
        memset(long_msg, 'A', NK_CONSOLE_MESSAGE_MAX_LENGTH + 45);
        long_msg[NK_CONSOLE_MESSAGE_MAX_LENGTH + 45] = '\0';
        nk_console_show_message(console, long_msg);
        nk_console_top_data* top_data = (nk_console_top_data*)(nk_console_get_top(console)->data);
        nk_console_message* last_msg = &top_data->messages[cvector_size(top_data->messages) - 1];
        assert(nk_strlen(last_msg->text) == NK_CONSOLE_MESSAGE_MAX_LENGTH);
    }

    // nk_console_message_slide_fraction() (#301)
    {
        nk_console_top_data* top_data = (nk_console_top_data*)(nk_console_get_top(console)->data);
        nk_console_message msg;
        memset(&msg, 0, sizeof(msg));

        // A non-animated message rests on screen at any duration.
        nk_bool saved_observed = top_data->message_time_observed;
        msg.animate = nk_false;
        msg.duration = NK_CONSOLE_MESSAGE_DURATION;
        assert(nk_console_message_slide_fraction(top_data, &msg) == 0.0f);
        msg.duration = 0.5f;
        assert(nk_console_message_slide_fraction(top_data, &msg) == 0.0f);

        // When animated, the fraction is a pure function of the duration, so a
        // zero-delta frame keeps the same offset instead of snapping to rest.
        msg.animate = nk_true;
        msg.duration = NK_CONSOLE_MESSAGE_DURATION;
        assert(nk_console_message_slide_fraction(top_data, &msg) == 1.0f);
        float mid_in = nk_console_message_slide_fraction(top_data, &msg);
        msg.duration = NK_CONSOLE_MESSAGE_DURATION - 0.5f;
        float half_in = nk_console_message_slide_fraction(top_data, &msg);
        assert(half_in > 0.0f && half_in < mid_in);
        msg.duration = NK_CONSOLE_MESSAGE_DURATION * 0.5f;
        assert(nk_console_message_slide_fraction(top_data, &msg) == 0.0f); // Resting.
        msg.duration = 0.5f;
        float half_out = nk_console_message_slide_fraction(top_data, &msg);
        assert(half_out > 0.0f && half_out < 1.0f);
        msg.duration = 0.0f;
        assert(nk_console_message_slide_fraction(top_data, &msg) == 1.0f);

        // Whether a message animates is decided when it is queued, so it can never
        // flip mid-flight and teleport from its resting spot to off screen (#301).
        top_data->message_time_observed = nk_false;
        nk_console_show_message(console, "Queued before any timing");
        assert(top_data->messages[cvector_size(top_data->messages) - 1].animate == nk_false);
        top_data->message_time_observed = nk_true;
        nk_console_show_message(console, "Queued once timing is available");
        assert(top_data->messages[cvector_size(top_data->messages) - 1].animate == nk_true);

        top_data->message_time_observed = saved_observed;
    }

    // nk_console_set_message_position() / nk_console_get_message_position()
    {
        assert(nk_console_get_message_position(console) == NK_CONSOLE_MESSAGE_POSITION_BOTTOM);
        nk_console_set_message_position(console, NK_CONSOLE_MESSAGE_POSITION_TOP);
        assert(nk_console_get_message_position(console) == NK_CONSOLE_MESSAGE_POSITION_TOP);
        nk_console_set_message_position(console, NK_CONSOLE_MESSAGE_POSITION_LEFT);
        assert(nk_console_get_message_position(console) == NK_CONSOLE_MESSAGE_POSITION_LEFT);
        nk_console_set_message_position(console, NK_CONSOLE_MESSAGE_POSITION_BOTTOM);
        assert(nk_console_get_message_position(console) == NK_CONSOLE_MESSAGE_POSITION_BOTTOM);
    }

    // nk_console_row()
    {
        nk_console* row = nk_console_row_begin(console);
        assert(row != NULL);
        nk_console_label(row, "Row Column 1")->alignment = NK_TEXT_CENTERED;
        nk_console_label(row, "Row Column 2")->alignment = NK_TEXT_CENTERED;
        nk_console_row_end(row);
    }

    // nk_console_selectable()
    {
        nk_console* widget = nk_console_label(console, "Selectable");
        widget->selectable = nk_false;
        widget->disabled = nk_false;
        assert(nk_console_selectable(widget) == nk_false);
        widget->selectable = nk_true;
        assert(nk_console_selectable(widget) == nk_true);
        widget->disabled = nk_true;
        assert(nk_console_selectable(widget) == nk_false);
        widget->selectable = nk_true;
        widget->disabled = nk_false;
        widget->visible = nk_true;
        assert(nk_console_selectable(widget) == nk_true);
        widget->visible = nk_false;
        assert(nk_console_selectable(widget) == nk_false);
    }

    // nk_console_knob_int/float()
    {
        static int knob_int_val = 5;
        nk_console* knob_int = nk_console_knob_int(console, "Knob Int", 0, &knob_int_val, 10, 1, 1.0f);
        assert(knob_int != NULL);
        static float knob_float_val = 0.5f;
        nk_console* knob_float = nk_console_knob_float(console, "Knob Float", 0.0f, &knob_float_val, 1.0f, 0.1f, 1.0f);
        assert(knob_float != NULL);
    }

    // NULL labels on progress, property, slider, knob and combobox widgets must render without crashing
    {
        nk_console* nolabels = nk_console_init(ctx);
        assert(nolabels != NULL);
        static nk_size progress_val = 10;
        static int int_val = 5;
        static float float_val = 0.5f;
        static int combobox_val = 0;
        nk_console* progress = nk_console_progress(nolabels, NULL, &progress_val, 100);
        assert(progress != NULL);
        nk_console_progress_update(progress, NULL, &progress_val, 50);
        assert(progress->label_length == 0);
        assert(progress->columns == 1);
        assert(nk_console_property_int(nolabels, NULL, 0, &int_val, 10, 1, 1) != NULL);
        assert(nk_console_property_float(nolabels, NULL, 0.0f, &float_val, 1.0f, 0.1f, 1) != NULL);
        assert(nk_console_slider_int(nolabels, NULL, 0, &int_val, 10, 1) != NULL);
        assert(nk_console_slider_float(nolabels, NULL, 0.0f, &float_val, 1.0f, 0.1f) != NULL);
        assert(nk_console_knob_int(nolabels, NULL, 0, &int_val, 10, 1, 1.0f) != NULL);
        assert(nk_console_knob_float(nolabels, NULL, 0.0f, &float_val, 1.0f, 0.1f, 1.0f) != NULL);
        assert(nk_console_combobox(nolabels, NULL, "A|B", '|', &combobox_val) != NULL);
        assert(nk_console_label(nolabels, NULL) != NULL);
        nk_console_render_window(nolabels, "nolabels", nk_rect(0, 0, 300, 800), NK_WINDOW_TITLE);
        nk_clear(ctx);
        nk_console_free(nolabels);
    }

    // nk_console_radio()
    {
        static int radio_selected = 1;
        nk_console* radio1 = nk_console_radio(console, "Option A", &radio_selected);
        nk_console* radio2 = nk_console_radio(console, "Option B", &radio_selected);
        nk_console* radio3 = nk_console_radio(console, "Option C", &radio_selected);
        assert(radio1 != NULL);
        assert(radio2 != NULL);
        assert(radio3 != NULL);
    }

    // nk_console_chart()
    {
        static const float chart_values[] = {0.1f, 0.5f, 0.3f, 0.8f, 0.2f, 0.9f};
        nk_console* chart = nk_console_chart(console, "Chart", NK_CHART_LINES, chart_values, 6, 0.0f, 1.0f);
        assert(chart != NULL);
        assert(chart->type == NK_CONSOLE_CHART);
        nk_console_chart_update(chart, chart_values, 6, 0.0f, 1.0f);
        nk_console* chart_col = nk_console_chart(console, NULL, NK_CHART_COLUMN, chart_values, 6, 0.0f, 1.0f);
        assert(chart_col != NULL);
        assert(chart->columns == 2);
        assert(chart_col->columns == 1);
        nk_console_chart_data* chart_data = (nk_console_chart_data*)chart->data;
        assert(chart_data->min_value == 0.0f && chart_data->max_value == 1.0f);
        nk_console_chart_update(chart, chart_values, 3, 0.0f, 100.0f);
        assert(chart_data->count == 3);
        assert(chart_data->max_value == 100.0f);

        // Taller chart
        nk_console_set_height(chart_col, 120);
        assert(nk_console_height(chart_col) == 120);
        assert(nk_console_layout_height(chart_col) == 120.0f);
    }

    // nk_console_rule_horizontal()
    {
        static struct nk_color rule_color = {200, 200, 200, 255};
        nk_console* rule = nk_console_rule_horizontal(console, rule_color, nk_false);
        assert(rule != NULL);
    }

    // nk_console_tree()
    {
        nk_console* tree = nk_console_tree(console, "Tree Node", nk_true);
        assert(tree != NULL);
        nk_console* tree_label = nk_console_label(tree, "Tree Child");
        assert(tree_label != NULL);
    }

    // nk_console_list_view()
    {
        nk_console* list_view = nk_console_list_view(console, "list_view", 3, 5, list_view_get_label);
        assert(list_view != NULL);
    }

    // nk_console_list_view_item_matches() - the search/filter predicate.
    {
        assert(nk_console_list_view_item_matches("Banana", "") == nk_true);       // empty filter matches everything
        assert(nk_console_list_view_item_matches("Banana", NULL) == nk_true);
        assert(nk_console_list_view_item_matches(NULL, "x") == nk_false);          // NULL label, non-empty filter
        assert(nk_console_list_view_item_matches("Banana", "ana") == nk_true);     // substring
        assert(nk_console_list_view_item_matches("Banana", "BANANA") == nk_true);  // case-insensitive
        assert(nk_console_list_view_item_matches("Banana", "nana") == nk_true);    // match at the end
        assert(nk_console_list_view_item_matches("Banana", "xyz") == nk_false);    // no match
        assert(nk_console_list_view_item_matches("item42", "42") == nk_true);      // digits compare verbatim
        assert(nk_console_list_view_item_matches("a[b", "{") == nk_false);         // ASCII fold only: '[' must not alias '{'
    }

    // nk_console_list_view_set_searchable() - inserts/removes the search field.
    {
        nk_console* lv = nk_console_list_view(console, "searchable_lv", 3, 5, list_view_get_label);
        assert(lv != NULL);
        nk_console_list_view_data* lv_data = (nk_console_list_view_data*)lv->data;

        int before = (int)cvector_size(console->children);
        nk_console_list_view_set_searchable(lv, nk_true);
        assert(lv_data->search != NULL);
        // The search field is inserted as a sibling, immediately before the list view.
        assert((int)cvector_size(console->children) == before + 1);
        assert(nk_console_get_widget_index(lv_data->search) == nk_console_get_widget_index(lv) - 1);

        // Enabling again must not add a second field.
        nk_console_list_view_set_searchable(lv, nk_true);
        assert((int)cvector_size(console->children) == before + 1);

        // Disabling removes the field and clears the filter.
        snprintf(lv_data->search_buffer, sizeof(lv_data->search_buffer), "abc");
        nk_console_list_view_set_searchable(lv, nk_false);
        assert(lv_data->search == NULL);
        assert(lv_data->search_buffer[0] == '\0');
        assert((int)cvector_size(console->children) == before);
    }

    // A key-driven frame before the screenshot must not hang the draw (#342).
    test_render_frame(console, NULL, NK_KEY_NONE);

    // nk_console_free() on a searchable list view also removes its search field.
    {
        int before = (int)cvector_size(console->children);
        nk_console* lv = nk_console_list_view(console, "free_searchable_lv", 3, 5, list_view_get_label);
        assert(lv != NULL);
        nk_console_list_view_set_searchable(lv, nk_true);
        assert((int)cvector_size(console->children) == before + 2);

        // Detach the list view from its parent and free it alone.
        int lv_idx = nk_console_get_widget_index(lv);
        assert(lv_idx >= 0);
        cvector_erase(console->children, (size_t)lv_idx);
        nk_console_free(lv);
        assert((int)cvector_size(console->children) == before);
        nk_console_render_window(console, "free_searchable_lv_test", nk_rect(0, 0, 300, 800), NK_WINDOW_TITLE);
    }

    // nk_console_color() - RGB and RGBA modes
    struct nk_colorf rgb_color = {1.0f, 0.0f, 0.0f, 1.0f};
    struct nk_colorf rgba_color = {0.0f, 1.0f, 0.0f, 0.5f};
    nk_console* color_rgb = nk_console_color(console, "Color RGB", &rgb_color, NK_RGB);
    assert(color_rgb != NULL);
    nk_console* color_rgba = nk_console_color(console, "Color RGBA", &rgba_color, NK_RGBA);
    assert(color_rgba != NULL);

    // nk_console_set_style() / nk_console_get_style() / nk_console_layout_height()
    {
        // The fonts and styles are shared by pointer, so they must outlive the widgets.
        static struct nk_user_font big_font;
        static struct nk_user_font small_font;
        static struct nk_style big_style;
        static struct nk_style small_style;

        // By default, no style is set: the context's own style is used.
        nk_console* style_parent = nk_console_button(console, "Styles");
        nk_console* style_child = nk_console_label(style_parent, "Style Label");
        assert(style_parent->style == NULL);
        assert(nk_console_get_style(style_parent) == NULL);
        assert(nk_console_get_style(style_child) == NULL);

        // Build the styles from the live context style, changing only what they need.
        big_font = *ctx->style.font;
        big_font.height *= 2.0f;
        big_style = ctx->style;
        big_style.font = &big_font;
        big_style.text.color = nk_rgb(255, 0, 0);

        small_font = *ctx->style.font;
        small_font.height *= 0.5f;
        small_style = ctx->style;
        small_style.font = &small_font;

        // Setting a style makes it the widget's effective style.
        nk_console_set_style(style_parent, &big_style);
        assert(style_parent->style == &big_style);
        assert(nk_console_get_style(style_parent) == &big_style);

        // Children inherit the closest ancestor's style...
        assert(nk_console_get_style(style_child) == &big_style);

        // ...unless they set their own.
        nk_console_set_style(style_child, &small_style);
        assert(style_child->style == &small_style);
        assert(nk_console_get_style(style_child) == &small_style);

        // Clearing a style restores inheritance.
        nk_console_set_style(style_child, NULL);
        assert(style_child->style == NULL);
        assert(nk_console_get_style(style_child) == &big_style);

        // Styles are inherited by default.
        assert(style_parent->style_inherits == nk_true);
        assert(nk_console_get_style_inherits(style_parent) == nk_true);

        // Disabling inheritance keeps the style to the widget itself.
        nk_console_set_style_inherits(style_parent, nk_false);
        assert(nk_console_get_style_inherits(style_parent) == nk_false);
        assert(nk_console_get_style(style_parent) == &big_style);
        assert(nk_console_get_style(style_child) == NULL);

        // A child's own style still applies.
        nk_console_set_style(style_child, &small_style);
        assert(nk_console_get_style(style_child) == &small_style);
        nk_console_set_style(style_child, NULL);

        // Children fall back to a further ancestor's style.
        nk_console_set_style(style_parent->parent, &small_style);
        assert(nk_console_get_style(style_child) == &small_style);
        nk_console_set_style(style_parent->parent, NULL);

        // Re-enabling inheritance restores it.
        nk_console_set_style_inherits(style_parent, nk_true);
        assert(nk_console_get_style(style_child) == &big_style);

        // nk_console_layout_height(): an explicit height wins.
        nk_console_set_height(style_child, 100);
        assert(nk_console_layout_height(style_child) == 100.0f);

        // Otherwise the row height is derived from the effective style's font and paddings.
        nk_console_set_height(style_child, 0);
        float expected = big_font.height + big_style.text.padding.y * 2.0f + big_style.window.min_row_height_padding * 2.0f;
        assert(nk_console_layout_height(style_child) == expected);

        // With no style and no height, 0 defers to the window's minimum row height.
        nk_console_set_style(style_parent, NULL);
        assert(nk_console_layout_height(style_child) == 0.0f);

        // Rows render their children directly, so style a row child to exercise that path.
        {
            nk_console* style_row = nk_console_row_begin(console);
            assert(style_row != NULL);
            nk_console_set_style(nk_console_label(style_row, "Styled Row Column"), &small_style);
            nk_console_label(style_row, "Default Row Column");
            nk_console_row_end(style_row);
        }

        // Leave the style set so the render passes below exercise the style swap.
        nk_console_set_style(style_parent, &big_style);
    }

    // Create the screen buffer
    pntr_image* screen = pntr_new_image(300, 800);
    assert(screen != NULL);

    // nk_console_render_window()
    nk_console_render_window(console, "nuklear_console_test", nk_rect(0, 0, (float)screen->width, (float)screen->height), NK_WINDOW_TITLE);

    // Draw the nuklear context on the screen.
    pntr_draw_nuklear(screen, ctx);

    // nk_console_color() - mutate and re-render to verify no crash and color update
    rgb_color.r = 0.0f;
    rgb_color.g = 0.5f;
    rgb_color.b = 1.0f;
    rgba_color.r = 1.0f;
    rgba_color.g = 0.0f;
    rgba_color.b = 0.5f;
    rgba_color.a = 0.8f;
    nk_console_render_window(console, "nuklear_console_color_test", nk_rect(0, 0, (float)screen->width, (float)screen->height), NK_WINDOW_TITLE);
    pntr_draw_nuklear(screen, ctx);

    // Save the output test image.
    pntr_save_image(screen, "nuklear_console_test.png");

    // List view search/filter behavior, driven through a real render pass.
    {
        nk_console* lv = nk_console_list_view(console, "filter_lv", 3, 5, list_view_get_label);
        assert(lv != NULL);
        nk_console_list_view_data* lv_data = (nk_console_list_view_data*)lv->data;
        nk_console_list_view_set_searchable(lv, nk_true);

        // Filter down to a single match ("Banana"); selection snaps onto it even
        // though it started on a now-hidden item.
        snprintf(lv_data->search_buffer, sizeof(lv_data->search_buffer), "an");
        lv_data->selected = 0; // Apple, which does not match.
        nk_console_render_window(console, "filter_test_1", nk_rect(0, 0, (float)screen->width, (float)screen->height), NK_WINDOW_TITLE);
        assert(lv_data->selected == 1); // Banana
        assert(strcmp(nk_console_list_view_selected_label(lv), "Banana") == 0);

        // A filter that matches nothing leaves the selection untouched and reports NULL.
        snprintf(lv_data->search_buffer, sizeof(lv_data->search_buffer), "zzz");
        lv_data->selected = 2; // Cherry
        nk_console_render_window(console, "filter_test_2", nk_rect(0, 0, (float)screen->width, (float)screen->height), NK_WINDOW_TITLE);
        assert(lv_data->selected == 2); // unchanged
        assert(nk_console_list_view_selected_label(lv) == NULL);

        // Clearing the filter restores normal selection reporting.
        lv_data->search_buffer[0] = '\0';
        nk_console_render_window(console, "filter_test_3", nk_rect(0, 0, (float)screen->width, (float)screen->height), NK_WINDOW_TITLE);
        assert(strcmp(nk_console_list_view_selected_label(lv), "Cherry") == 0);
    }

    // Row navigation skips disabled and hidden children.
    {
        nk_console* nav = nk_console_init(ctx);
        assert(nav != NULL);
        #ifndef NK_CONSOLE_NO_GAMEPAD
        nk_console_set_gamepads(nav, &gamepads);
        #endif
        nk_console* row = nk_console_row_begin(nav);
        nk_console* first = nk_console_button(row, "First");
        nk_console* disabled = nk_console_button(row, "Disabled");
        nk_console* hidden = nk_console_button(row, "Hidden");
        nk_console* last = nk_console_button(row, "Last");
        nk_console_set_disabled(disabled, nk_true);
        hidden->visible = nk_false;
        nk_console_row_end(row);
        nk_console_row_data* row_data = (nk_console_row_data*)row->data;
        nk_console_set_active_widget(row);
        assert(row_data->activeChild == 0);

        // Pressing Right from the first child lands on the last, skipping the disabled and hidden ones.
        nk_input_begin(ctx);
        nk_input_key(ctx, NK_KEY_RIGHT, nk_true);
        nk_input_key(ctx, NK_KEY_RIGHT, nk_false);
        nk_input_end(ctx);
        nk_console_render_window(nav, "row_nav_right", nk_rect(0, 0, (float)screen->width, (float)screen->height), NK_WINDOW_TITLE);
        nk_clear(ctx);
        assert(row->children[row_data->activeChild] == last);

        // Pressing Left goes back to the first child.
        nk_input_begin(ctx);
        nk_input_key(ctx, NK_KEY_LEFT, nk_true);
        nk_input_key(ctx, NK_KEY_LEFT, nk_false);
        nk_input_end(ctx);
        nk_console_render_window(nav, "row_nav_left", nk_rect(0, 0, (float)screen->width, (float)screen->height), NK_WINDOW_TITLE);
        nk_clear(ctx);
        assert(row->children[row_data->activeChild] == first);

        // A row starting on a disabled child picks the nearest navigable one.
        nk_console* row2 = nk_console_row_begin(nav);
        nk_console_set_disabled(nk_console_button(row2, "Disabled"), nk_true);
        nk_console* enabled = nk_console_button(row2, "Enabled");
        nk_console_row_end(row2);
        assert(row2->children[((nk_console_row_data*)row2->data)->activeChild] == enabled);

        nk_input_begin(ctx);
        nk_input_end(ctx);
        nk_console_free(nav);
    }

    // nk_console_marquee_slice()
    {
        const char* text = "Hello, Marquee!";
        int text_len = (int)strlen(text);
        float full_width = ctx->style.font->width(ctx->style.font->userdata, ctx->style.font->height, text, text_len);
        float speed = 60.0f;
        float pause = 1.5f;
        float scroll_x;
        float shift;
        char buf[256];

        // Text fits in available width: returned unchanged.
        scroll_x = 0.0f;
        const char* result = nk_console_marquee_slice(ctx, text, text_len, full_width, full_width + 10.0f, speed, pause, &scroll_x, buf, sizeof(buf), &shift);
        assert(result == text);
        assert(scroll_x == 0.0f);
        assert(shift == 0.0f);

        // Zero delta time with no scroll accumulated yet: returned unchanged.
        float saved_dt = ctx->delta_time_seconds;
        ctx->delta_time_seconds = 0.0f;
        scroll_x = 0.0f;
        result = nk_console_marquee_slice(ctx, text, text_len, full_width, full_width * 0.5f, speed, pause, &scroll_x, buf, sizeof(buf), &shift);
        assert(result == text);
        assert(scroll_x == 0.0f);
        ctx->delta_time_seconds = saved_dt;

        // Pause window: scroll advances but offset is still negative, full text returned.
        float avail = full_width * 0.5f;
        ctx->delta_time_seconds = 0.1f;
        scroll_x = 0.0f;
        result = nk_console_marquee_slice(ctx, text, text_len, full_width, avail, speed, pause, &scroll_x, buf, sizeof(buf), &shift);
        assert(scroll_x > 0.0f);
        assert(result == text);

        // After enough time, text starts scrolling (returned slice differs from text).
        scroll_x = 0.0f;
        ctx->delta_time_seconds = pause + 0.5f;
        result = nk_console_marquee_slice(ctx, text, text_len, full_width, avail, speed, pause, &scroll_x, buf, sizeof(buf), &shift);
        assert(result == buf);
        assert(strlen(result) > 0);

        // A zero delta frame keeps the scrolled slice instead of snapping back to
        // the start of the text, so high frame rates don't strobe the marquee.
        float scrolled_x = scroll_x;
        float scrolled_shift = shift;
        char scrolled[256];
        strcpy(scrolled, result);
        ctx->delta_time_seconds = 0.0f;
        result = nk_console_marquee_slice(ctx, text, text_len, full_width, avail, speed, pause, &scroll_x, buf, sizeof(buf), &shift);
        assert(result == buf);
        assert(strcmp(result, scrolled) == 0);
        assert(scroll_x == scrolled_x);
        assert(shift == scrolled_shift);

        // Wrap-around: scroll_x exceeding total_cycle gets wrapped.
        float pause_pixels = pause * speed;
        float total_cycle = full_width + pause_pixels;
        scroll_x = total_cycle - 1.0f;
        ctx->delta_time_seconds = 2.0f / speed;
        result = nk_console_marquee_slice(ctx, text, text_len, full_width, avail, speed, pause, &scroll_x, buf, sizeof(buf), &shift);
        assert(scroll_x < total_cycle);

        // The reported shift is the sub-character remainder of the scroll offset.
        scroll_x = pause_pixels + 1.0f;
        ctx->delta_time_seconds = 0.0f;
        result = nk_console_marquee_slice(ctx, text, text_len, full_width, avail, speed, pause, &scroll_x, buf, sizeof(buf), &shift);
        assert(result == buf);
        assert(shift >= 0.0f);
        assert(shift <= 1.0f); // First glyph hasn't fully scrolled off yet.

        // Small buffer: output is truncated safely.
        char tiny_buf[4];
        scroll_x = pause_pixels + 1.0f;
        ctx->delta_time_seconds = 0.01f;
        result = nk_console_marquee_slice(ctx, text, text_len, full_width, avail, speed, pause, &scroll_x, tiny_buf, sizeof(tiny_buf), &shift);
        assert(strlen(result) <= 3);

        // A NULL shift pointer is allowed.
        scroll_x = pause_pixels + 1.0f;
        result = nk_console_marquee_slice(ctx, text, text_len, full_width, avail, speed, pause, &scroll_x, buf, sizeof(buf), NULL);
        assert(result == buf);

        ctx->delta_time_seconds = saved_dt;
    }

    // nk_console_textedit_set_filter()
    {
        static char filter_buffer[32] = "";
        nk_console* filtered = nk_console_textedit(console, "Filtered", filter_buffer, (int)sizeof(filter_buffer));
        assert(filtered != NULL);

        // ASCII entry is the default.
        assert(nk_console_textedit_get_filter(filtered) == nk_filter_ascii);

        // NULL restores the default.
        nk_console_textedit_set_filter(filtered, nk_filter_hex);
        assert(nk_console_textedit_get_filter(filtered) == nk_filter_hex);
        nk_console_textedit_set_filter(filtered, NULL);
        assert(nk_console_textedit_get_filter(filtered) == nk_filter_ascii);

        // Restrict the widget to decimal digits, then build the on-screen keyboard.
        nk_console_textedit_set_filter(filtered, nk_filter_decimal);
        nk_console_textedit_button_main_click(filtered, NULL);

        nk_console* digit = NULL;
        nk_console* letter = NULL;
        nk_console* shift = NULL;
        for (int i = 0; i < (int)cvector_size(filtered->children); i++) {
            nk_console* row = filtered->children[i];
            if (row->type != NK_CONSOLE_ROW) {
                continue;
            }
            for (int k = 0; k < (int)cvector_size(row->children); k++) {
                nk_console* key = row->children[k];
                if (nk_console_button_get_symbol(key) == NK_SYMBOL_TRIANGLE_UP_OUTLINE) {
                    shift = key;
                }
                else if (key->label != NULL && strcmp(key->label, "5") == 0) {
                    digit = key;
                }
                else if (key->label != NULL && strcmp(key->label, "q") == 0) {
                    letter = key;
                }
            }
        }
        assert(digit != NULL && letter != NULL && shift != NULL);

        // Keys the filter rejects are shown as disabled; allowed keys are not.
        assert(nk_console_is_disabled(digit) == nk_false);
        assert(nk_console_is_disabled(letter) == nk_true);

        // Pressing a rejected key does nothing, while an allowed key is appended.
        nk_console_textedit_key_click(letter, NULL);
        assert(strcmp(filter_buffer, "") == 0);
        nk_console_textedit_key_click(digit, NULL);
        assert(strcmp(filter_buffer, "5") == 0);

        // Toggling shift re-evaluates the filter against the new labels: "5" becomes "%".
        nk_console_textedit_key_click(shift, NULL);
        assert(strcmp(digit->label, "%") == 0);
        assert(nk_console_is_disabled(digit) == nk_true);
        nk_console_textedit_key_click(digit, NULL);
        assert(strcmp(filter_buffer, "5") == 0);

        // Restore the console state for the remaining tests.
        nk_console_free_children(filtered);
        nk_console_set_active_parent(console);
    }

    // nk_console_textedit_masked() edits, driven through real render passes.
    {
        struct nk_console* masked_console = nk_console_init(ctx);
        assert(masked_console != NULL);

        // A masked buffer holding more than 1024 characters isn't truncated by rendering.
        static char big_buffer[2048];
        for (int i = 0; i < 1500; i++) {
            big_buffer[i] = (char)('a' + i % 26);
        }
        big_buffer[1500] = '\0';
        nk_console* big = nk_console_textedit_masked(masked_console, "Big", big_buffer, (int)sizeof(big_buffer));
        assert(big != NULL);
        nk_console_textedit_button_main_click(big, NULL);
        assert(big->children[0]->type == NK_CONSOLE_TEXTEDIT_TEXT);
        nk_console_set_active_widget(big->children[0]);
        for (int frame = 0; frame < 3; frame++) {
            test_render_frame(masked_console, NULL, NK_KEY_NONE);
        }
        assert(strlen(big_buffer) == 1500);
        assert(big_buffer[1499] == (char)('a' + 1499 % 26));
        assert(((nk_console_textedit_data*)big->data)->changed == nk_false);

        // Typing past 1024 characters appends to the real buffer.
        test_render_frame(masked_console, NULL, NK_KEY_TEXT_END);
        test_render_frame(masked_console, "Z", NK_KEY_NONE);
        assert(strlen(big_buffer) == 1501);
        assert(big_buffer[1500] == 'Z');
        assert(((nk_console_textedit_data*)big->data)->changed == nk_true);
        nk_console_free_children(big);
        nk_console_set_active_parent(masked_console);

        // Edits are applied at the cursor, not at the end of the real buffer.
        static char edit_buffer[32] = "abcd";
        nk_console* edit = nk_console_textedit_masked(masked_console, "Edit", edit_buffer, (int)sizeof(edit_buffer));
        assert(edit != NULL);
        nk_console_textedit_button_main_click(edit, NULL);
        nk_console_set_active_widget(edit->children[0]);
        test_render_frame(masked_console, NULL, NK_KEY_NONE);
        assert(strcmp(edit_buffer, "abcd") == 0);
        test_render_frame(masked_console, NULL, NK_KEY_TEXT_END);
        test_render_frame(masked_console, NULL, NK_KEY_LEFT);
        test_render_frame(masked_console, NULL, NK_KEY_LEFT);
        test_render_frame(masked_console, "X", NK_KEY_NONE);
        assert(strcmp(edit_buffer, "abXcd") == 0);

        // Typing '*' is inserted at the cursor too.
        test_render_frame(masked_console, "*", NK_KEY_NONE);
        assert(strcmp(edit_buffer, "abX*cd") == 0);

        // Backspace removes the character before the cursor.
        test_render_frame(masked_console, NULL, NK_KEY_BACKSPACE);
        assert(strcmp(edit_buffer, "abXcd") == 0);
        test_render_frame(masked_console, NULL, NK_KEY_BACKSPACE);
        assert(strcmp(edit_buffer, "abcd") == 0);

        // Delete removes the character after the cursor.
        test_render_frame(masked_console, NULL, NK_KEY_DEL);
        assert(strcmp(edit_buffer, "abd") == 0);

        // Typing at the start inserts at the start.
        test_render_frame(masked_console, NULL, NK_KEY_TEXT_START);
        test_render_frame(masked_console, "Y", NK_KEY_NONE);
        assert(strcmp(edit_buffer, "Yabd") == 0);
        nk_console_free_children(edit);
        nk_console_set_active_parent(masked_console);

        // Multi-byte UTF-8 characters are masked and deleted as whole glyphs.
        static char utf8_buffer[32] = "a\xc3\xa9\xe2\x82\xac"; // "a", U+00E9, U+20AC
        nk_console* utf8 = nk_console_textedit_masked(masked_console, "UTF-8", utf8_buffer, (int)sizeof(utf8_buffer));
        assert(utf8 != NULL);
        nk_console_textedit_set_filter(utf8, nk_filter_default);
        nk_console_textedit_button_main_click(utf8, NULL);
        nk_console_set_active_widget(utf8->children[0]);
        test_render_frame(masked_console, NULL, NK_KEY_NONE);
        assert(strcmp(utf8_buffer, "a\xc3\xa9\xe2\x82\xac") == 0);
        test_render_frame(masked_console, NULL, NK_KEY_TEXT_END);
        test_render_frame(masked_console, NULL, NK_KEY_BACKSPACE);
        assert(strcmp(utf8_buffer, "a\xc3\xa9") == 0);
        test_render_frame(masked_console, "\xc3\xb1", NK_KEY_NONE); // U+00F1
        assert(strcmp(utf8_buffer, "a\xc3\xa9\xc3\xb1") == 0);
        test_render_frame(masked_console, NULL, NK_KEY_LEFT);
        test_render_frame(masked_console, NULL, NK_KEY_BACKSPACE);
        assert(strcmp(utf8_buffer, "a\xc3\xb1") == 0);
        nk_console_free_children(utf8);
        nk_console_set_active_parent(masked_console);

        // A full buffer with multi-byte characters can't overflow through the mask.
        static char full_buffer[6] = "\xc3\xa9\xc3\xa9"; // Two U+00E9, 4 bytes plus terminator.
        nk_console* full = nk_console_textedit_masked(masked_console, "Full", full_buffer, (int)sizeof(full_buffer));
        assert(full != NULL);
        nk_console_textedit_set_filter(full, nk_filter_default);
        nk_console_textedit_button_main_click(full, NULL);
        nk_console_set_active_widget(full->children[0]);
        test_render_frame(masked_console, NULL, NK_KEY_TEXT_END);
        test_render_frame(masked_console, "x\xc3\xb1", NK_KEY_NONE);
        assert(strcmp(full_buffer, "\xc3\xa9\xc3\xa9x") == 0);
        test_render_frame(masked_console, "\xc3\xb1", NK_KEY_NONE);
        assert(strcmp(full_buffer, "\xc3\xa9\xc3\xa9x") == 0);

        nk_console_free(masked_console);
    }

    // nk_console_widget_type_name()
    {
        // Known values map to their lowercase names.
        assert(strcmp(nk_console_widget_type_name(NK_CONSOLE_BUTTON), "button") == 0);
        assert(strcmp(nk_console_widget_type_name(NK_CONSOLE_LABEL), "label") == 0);
        assert(strcmp(nk_console_widget_type_name(NK_CONSOLE_UNKNOWN), "unknown") == 0);
        assert(strcmp(nk_console_widget_type_name(NK_CONSOLE_CHART), "chart") == 0);

        // Out-of-range values fall back to "unknown".
        assert(strcmp(nk_console_widget_type_name((nk_console_widget_type)(NK_CONSOLE_TAB + 1)), "unknown") == 0);

        // Every widget type has a non-NULL name.
        for (int type = NK_CONSOLE_UNKNOWN; type <= NK_CONSOLE_TAB; type++) {
            assert(nk_console_widget_type_name((nk_console_widget_type)type) != NULL);
        }
    }

    // nk_console_tabs()
    {
        nk_console* screen = nk_console_button(console, "Tabs Screen");
        nk_console* above = nk_console_button(screen, "Above");
        nk_console* tabs = nk_console_tabs(screen);
        assert(tabs != NULL);
        assert(tabs->type == NK_CONSOLE_TABS);
        assert(strcmp(nk_console_widget_type_name(NK_CONSOLE_TABS), "tabs") == 0);
        nk_console* tab_a = nk_console_tab(tabs, "A");
        nk_console* tab_b = nk_console_tab(tabs, "B");
        nk_console* tab_c = nk_console_tab(tabs, "C");
        assert(tab_a != NULL && tab_b != NULL && tab_c != NULL);
        assert(nk_console_tabs_selected_index(tabs) == 0);

        // Widgets are siblings of the tabs widget, and only the selected tab's are visible.
        nk_console* a1 = nk_console_button(tab_a, "A1");
        nk_console* a2 = nk_console_button(tab_a, "A2");
        nk_console* b1 = nk_console_button(tab_b, "B1");
        nk_console* c1 = nk_console_button(tab_c, "C1");
        nk_console* below = nk_console_button(screen, "Below");
        assert(a1->parent == screen && b1->parent == screen);
        assert(cvector_size(screen->children) == 7);
        assert(nk_console_get_widget_index(tabs) == 1);
        assert(nk_console_get_widget_index(below) == 6);
        assert(nk_console_is_visible(a1) && nk_console_is_visible(a2) && !nk_console_is_visible(b1) && !nk_console_is_visible(c1) && nk_console_is_visible(below));

        // Adding to an earlier tab keeps the tab's widgets before the widgets after the tabs.
        nk_console* a3 = nk_console_button(tab_a, "A3");
        assert(nk_console_get_widget_index(below) == 7);
        assert(nk_console_get_widget_index(a3) < nk_console_get_widget_index(below));

        // Selecting a tab shows its widgets and triggers the changed event.
        int changed = 0;
        nk_console_add_event_handler(tabs, NK_CONSOLE_EVENT_CHANGED, &test_event_counter, &changed, NULL);
        nk_console_tabs_set_selected_index(tabs, 1);
        assert(nk_console_tabs_selected_index(tabs) == 1);
        assert(!nk_console_is_visible(a1) && nk_console_is_visible(b1) && !nk_console_is_visible(c1));
        assert(changed == 1);
        nk_console_tabs_set_selected_index(tabs, 1);
        nk_console_tabs_set_selected_index(tabs, 99);
        assert(nk_console_tabs_selected_index(tabs) == 1);
        assert(changed == 1);
        nk_console_tabs_set_selected_index(tabs, 0);

        // Tabs can be selected and checked directly.
        assert(nk_console_tabs_get_selected_tab(tabs) == tab_a);
        assert(nk_console_tabs_selected(tab_a) && !nk_console_tabs_selected(tab_c));
        nk_console_tabs_set_selected(tab_c);
        assert(nk_console_tabs_selected(tab_c) && !nk_console_tabs_selected(tab_a));
        assert(nk_console_tabs_selected_index(tabs) == 2);
        assert(nk_console_is_visible(c1) && !nk_console_is_visible(a1));
        assert(changed == 3);
        nk_console_tabs_set_selected(tab_a);

        // LB/RB (Shift+Up/Down) switch tabs, wrapping around the ends, and focus follows into the tab.
        nk_console_set_active_parent(screen);
        nk_console_set_active_widget(a2);
        test_tap_key(console, NK_KEY_DOWN, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 1);
        assert(screen->activeWidget == b1);
        test_tap_key(console, NK_KEY_DOWN, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 2);
        test_tap_key(console, NK_KEY_DOWN, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 0);

        // The last active widget in a tab is restored.
        assert(screen->activeWidget == a2);
        test_tap_key(console, NK_KEY_UP, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 2);
        assert(screen->activeWidget == c1);

        // Disabled tabs are skipped.
        nk_console_set_disabled(tab_a, nk_true);
        test_tap_key(console, NK_KEY_DOWN, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 1);
        nk_console_set_disabled(tab_a, nk_false);

        // LB/RB don't switch a disabled tabs widget, and don't page either.
        nk_console_set_active_widget(b1);
        nk_console_set_disabled(tabs, nk_true);
        test_tap_key(console, NK_KEY_DOWN, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 1);
        assert(screen->activeWidget == b1);
        nk_console_set_disabled(tabs, nk_false);

        // Left/Right switch tabs while the tab headers are focused, keeping focus on the headers.
        nk_console_set_active_widget(tabs);
        test_tap_key(console, NK_KEY_RIGHT, nk_false);
        assert(nk_console_tabs_selected_index(tabs) == 2);
        assert(screen->activeWidget == tabs);
        test_tap_key(console, NK_KEY_LEFT, nk_false);
        assert(nk_console_tabs_selected_index(tabs) == 1);

        // LB/RB switch tabs while the tab headers are focused too.
        test_tap_key(console, NK_KEY_DOWN, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 2);
        assert(screen->activeWidget == tabs);
        test_tap_key(console, NK_KEY_UP, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 1);

        // Outside of the tabs, LB/RB page rather than switching tabs.
        nk_console_set_active_widget(above);
        test_tap_key(console, NK_KEY_DOWN, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 1);
        assert(screen->activeWidget == below);
        test_tap_key(console, NK_KEY_UP, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 1);
        assert(screen->activeWidget == above);

        // Without tabs, LB/RB still page.
        nk_console* no_tabs = nk_console_button(console, "No Tabs Screen");
        nk_console* first = nk_console_button(no_tabs, "First");
        nk_console* second = nk_console_button(no_tabs, "Second");
        nk_console_set_active_parent(no_tabs);
        nk_console_set_active_widget(first);
        test_tap_key(console, NK_KEY_DOWN, nk_true);
        assert(no_tabs->activeWidget == second);
        nk_console_set_active_parent(screen);

        nk_console_tabs_set_selected_index(tabs, 2);

        // Freeing a widget within a tab removes it from the tab.
        nk_console_free(a3);
        assert(cvector_size(((nk_console_tab_data*)tab_a->data)->referenced_children) == 2);

        // Freeing the selected tab frees its widgets, and selects the first remaining tab.
        size_t before = cvector_size(screen->children);
        nk_console_free(tab_c);
        assert(cvector_size(screen->children) == before - 1);
        test_render_frame(console, NULL, NK_KEY_NONE);
        assert(nk_console_tabs_selected_index(tabs) == 0);
        assert(nk_console_is_visible(a1) && !nk_console_is_visible(b1));

        // Freeing the tabs widget frees all its tabs' widgets.
        nk_console_free(tabs);
        assert(cvector_size(screen->children) == 2);
        assert(screen->children[0] == above && screen->children[1] == below);
        test_render_frame(console, NULL, NK_KEY_NONE);

        // Freeing an earlier tab keeps the same tab selected.
        nk_console* free_screen = nk_console_button(console, "Tabs Free Screen");
        nk_console* free_tabs = nk_console_tabs(free_screen);
        nk_console* tab_x = nk_console_tab(free_tabs, "X");
        nk_console* tab_y = nk_console_tab(free_tabs, "Y");
        nk_console* tab_z = nk_console_tab(free_tabs, "Z");
        nk_console_button(tab_x, "X1");
        nk_console* y1 = nk_console_button(tab_y, "Y1");
        nk_console* z1 = nk_console_button(tab_z, "Z1");
        nk_console_tabs_set_selected_index(free_tabs, 1);
        nk_console_free(tab_x);
        assert(nk_console_tabs_selected_index(free_tabs) == 0);
        assert(nk_console_is_visible(y1) && !nk_console_is_visible(z1));
        nk_console_set_active_parent(free_screen);
        test_render_frame(console, NULL, NK_KEY_NONE);
        assert(nk_console_tabs_selected_index(free_tabs) == 0);
        assert(nk_console_is_visible(y1) && !nk_console_is_visible(z1));

        // Freeing the selected tab selects the first remaining one.
        nk_console_free(tab_y);
        assert(nk_console_tabs_get_selected_tab(free_tabs) == tab_z);
        assert(nk_console_is_visible(z1));
        nk_console_set_active_parent(console);
        nk_console_free(free_screen);

        // Freeing a screen with tabs frees everything.
        nk_console* tabs2 = nk_console_tabs(screen);
        nk_console_button(nk_console_tab(tabs2, "X"), "X1");
        nk_console_button(nk_console_tab(tabs2, "Y"), "Y1");
        nk_console_set_active_parent(console);
        nk_console_free(screen);
        nk_console_free(no_tabs);
        test_render_frame(console, NULL, NK_KEY_NONE);
    }

    // Containers hide widgets without touching the user's visible flag.
    {
        nk_console* screen = nk_console_button(console, "Container Visibility Screen");
        nk_console_set_active_parent(screen);

        // A widget the user hid within a tab stays hidden through tab switches.
        nk_console* tabs = nk_console_tabs(screen);
        nk_console* tab_a = nk_console_tab(tabs, "A");
        nk_console* tab_b = nk_console_tab(tabs, "B");
        nk_console* a1 = nk_console_button(tab_a, "A1");
        nk_console* a_hidden = nk_console_button(tab_a, "A Hidden");
        nk_console* b1 = nk_console_button(tab_b, "B1");
        a_hidden->visible = nk_false;
        assert(b1->visible && !nk_console_is_visible(b1));
        nk_console_tabs_set_selected_index(tabs, 1);
        assert(a1->visible && !nk_console_is_visible(a1) && nk_console_is_visible(b1));
        nk_console_tabs_set_selected_index(tabs, 0);
        assert(nk_console_is_visible(a1) && !a_hidden->visible && !nk_console_is_visible(a_hidden));
        assert(!nk_console_selectable(a_hidden));
        a_hidden->visible = nk_true;
        assert(nk_console_is_visible(a_hidden));

        // A Tree within a tab hides its children along with the tab.
        nk_console* tree = nk_console_tree(tab_b, "Tree", nk_true);
        nk_console* tree_child = nk_console_button(tree, "Tree Child");
        nk_console* tree_hidden = nk_console_button(tree, "Tree Hidden");
        nk_console* b2 = nk_console_button(tab_b, "B2");
        tree_hidden->visible = nk_false;
        assert(!nk_console_is_visible(tree) && !nk_console_is_visible(tree_child));

        // Widgets added to the tab after the tree sit after the tree's children.
        assert(nk_console_get_widget_index(tree_child) == nk_console_get_widget_index(tree) + 1);
        assert(nk_console_get_widget_index(b2) == nk_console_get_widget_index(tree) + 3);
        nk_console_tabs_set_selected_index(tabs, 1);
        assert(nk_console_is_visible(tree) && nk_console_is_visible(tree_child) && !nk_console_is_visible(tree_hidden));

        // A collapsed tree keeps its children hidden when its tab is selected.
        nk_console_tree_set_expanded(tree, nk_false);
        assert(!nk_console_is_visible(tree_child) && tree_child->visible);
        nk_console_tabs_set_selected_index(tabs, 0);
        nk_console_tabs_set_selected_index(tabs, 1);
        assert(nk_console_is_visible(tree) && !nk_console_is_visible(tree_child));

        // Expanding a tree in an unselected tab keeps its children hidden, and a user-hidden child stays hidden.
        nk_console_tabs_set_selected_index(tabs, 0);
        nk_console_tree_set_expanded(tree, nk_true);
        assert(!nk_console_is_visible(tree_child));
        nk_console_tabs_set_selected_index(tabs, 1);
        assert(nk_console_is_visible(tree_child) && !nk_console_is_visible(tree_hidden));
        assert(!tree_hidden->visible);

        // LB/RB from a tree child within a tab switch tabs, and focus returns to it.
        nk_console_set_active_widget(tree_child);
        test_tap_key(console, NK_KEY_DOWN, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 0);
        assert(screen->activeWidget == a1);
        test_tap_key(console, NK_KEY_DOWN, nk_true);
        assert(nk_console_tabs_selected_index(tabs) == 1);
        assert(screen->activeWidget == tree_child);

        // Hiding the tree hides its children, and hiding the tabs widget hides all its tabs' widgets.
        tree->visible = nk_false;
        assert(!nk_console_is_visible(tree_child) && nk_console_is_visible(b2));
        tree->visible = nk_true;
        tabs->visible = nk_false;
        assert(!nk_console_is_visible(b2) && !nk_console_is_visible(tree_child));
        tabs->visible = nk_true;
        assert(nk_console_is_visible(b2) && nk_console_is_visible(tree_child));

        // A searchable List View within a tab shows its search field only along with the tab.
        nk_console* list_view = nk_console_list_view(tab_a, "tab_list_view", 3, 5, list_view_get_label);
        nk_console_list_view_set_searchable(list_view, nk_true);
        nk_console* search = ((nk_console_list_view_data*)list_view->data)->search;
        assert(search != NULL);
        assert(nk_console_get_widget_index(search) == nk_console_get_widget_index(list_view) - 1);
        assert(!nk_console_is_visible(list_view) && !nk_console_is_visible(search));
        nk_console_tabs_set_selected_index(tabs, 0);
        assert(nk_console_is_visible(list_view) && nk_console_is_visible(search));
        test_render_frame(console, NULL, NK_KEY_NONE);

        // Freeing the tab with the tree frees the tree's children too.
        size_t before = cvector_size(screen->children);
        nk_console_free(tab_b);
        assert(cvector_size(screen->children) == before - 5);
        test_render_frame(console, NULL, NK_KEY_NONE);

        nk_console_set_active_parent(console);
        nk_console_free(screen);
        test_render_frame(console, NULL, NK_KEY_NONE);
    }

    // A widget the user hid within a tree stays hidden through expanding and collapsing.
    {
        nk_console* tree = nk_console_tree(console, "User Hidden Tree", nk_false);
        nk_console* shown = nk_console_label(tree, "Shown");
        nk_console* hidden = nk_console_label(tree, "Hidden");
        hidden->visible = nk_false;
        assert(shown->visible && !nk_console_is_visible(shown) && !nk_console_is_visible(hidden));
        nk_console_tree_set_expanded(tree, nk_true);
        assert(nk_console_is_visible(shown) && !nk_console_is_visible(hidden) && !hidden->visible);
        nk_console_tree_set_expanded(tree, nk_false);
        nk_console_tree_set_expanded(tree, nk_true);
        assert(!nk_console_is_visible(hidden) && !hidden->visible);
        nk_console_free(tree);
    }

    // nk_console_remove_event_handler()
    {
        nk_console* label = nk_console_label(console, "Remove Event Handler");
        int count_a = 0;
        int count_b = 0;
        nk_console_add_event_handler(label, NK_CONSOLE_EVENT_CLICKED, &test_event_counter, &count_a, NULL);
        nk_console_add_event_handler(label, NK_CONSOLE_EVENT_CLICKED, &test_event_counter, &count_b, NULL);
        nk_console_trigger_event(label, NK_CONSOLE_EVENT_CLICKED);
        assert(count_a == 1 && count_b == 1);

        // Only the handler matching the user data is removed.
        nk_console_remove_event_handler(label, NK_CONSOLE_EVENT_CLICKED, &test_event_counter, &count_a);
        nk_console_trigger_event(label, NK_CONSOLE_EVENT_CLICKED);
        assert(count_a == 1 && count_b == 2);

        // Handlers for other event types are left alone.
        nk_console_remove_event_handler(label, NK_CONSOLE_EVENT_CHANGED, &test_event_counter, &count_b);
        nk_console_trigger_event(label, NK_CONSOLE_EVENT_CLICKED);
        assert(count_b == 3);
        nk_console_free(label);
    }

    // NK_CONSOLE_EVENT_POST_RENDER_ONCE
    {
        nk_console* label = nk_console_label(console, "Post Render Once");
        test_post_render_once_data data = {label, 0, 0};
        nk_console_add_event_handler(label, NK_CONSOLE_EVENT_POST_RENDER_ONCE, test_post_render_once_callback, &data, test_post_render_once_destructor);

        // The handler receives the widget and the caller's user_data, then is destroyed.
        nk_console_render_window(console, "post_render_once_1", nk_rect(0, 0, 300, 800), NK_WINDOW_TITLE);
        assert(data.callback_count == 1);
        assert(data.destructor_count == 1);

        // It only runs once.
        nk_console_render_window(console, "post_render_once_2", nk_rect(0, 0, 300, 800), NK_WINDOW_TITLE);
        assert(data.callback_count == 1);
        assert(data.destructor_count == 1);
    }

    // Unload
    nk_console_free(console);
    #ifndef NK_CONSOLE_NO_GAMEPAD
    nk_gamepad_free(&gamepads);
    #endif
    pntr_unload_image(image_value);
    pntr_unload_nuklear(ctx);
    pntr_unload_image(screen);
    pntr_unload_font(font);

    printf("nuklear_console_test: Tests passed.\n");

    return 0;
}
