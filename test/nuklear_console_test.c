// This test suite uses pntr_nuklear to render the Nuklear Context to an image.
// https://github.com/RobLoach/pntr_nuklear

#include <assert.h>
#include <stdio.h>

#define PNTR_ENABLE_DEFAULT_FONT
#define PNTR_IMPLEMENTATION
#include "pntr.h"

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

static const char* list_view_get_label(struct nk_console* list_view, nk_uint index) {
    (void)list_view;
    static const char* items[] = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};
    if (index < 5) {
        return items[index];
    }
    return NULL;
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

        // NULL safety.
        assert(nk_console_message_slide_fraction(NULL, &msg) == 0.0f);
        assert(nk_console_message_slide_fraction(top_data, NULL) == 0.0f);

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

        // NULL widgets are safe no-ops.
        nk_console_set_style(NULL, NULL);
        assert(nk_console_get_style(NULL) == NULL);
        assert(nk_console_layout_height(NULL) == 0.0f);

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

    // nk_console_widget_type_name()
    {
        // Known values map to their lowercase names.
        assert(strcmp(nk_console_widget_type_name(NK_CONSOLE_BUTTON), "button") == 0);
        assert(strcmp(nk_console_widget_type_name(NK_CONSOLE_LABEL), "label") == 0);
        assert(strcmp(nk_console_widget_type_name(NK_CONSOLE_UNKNOWN), "unknown") == 0);
        assert(strcmp(nk_console_widget_type_name(NK_CONSOLE_CHART), "chart") == 0);

        // Out-of-range values fall back to "unknown".
        assert(strcmp(nk_console_widget_type_name((nk_console_widget_type)(NK_CONSOLE_CHART + 1)), "unknown") == 0);

        // Every widget type has a non-NULL name.
        for (int type = NK_CONSOLE_UNKNOWN; type <= NK_CONSOLE_CHART; type++) {
            assert(nk_console_widget_type_name((nk_console_widget_type)type) != NULL);
        }
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
