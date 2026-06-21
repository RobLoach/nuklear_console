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
