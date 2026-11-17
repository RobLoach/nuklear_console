#ifndef NK_CONSOLE_TEXTEDIT_TEXT_H__
#define NK_CONSOLE_TEXTEDIT_TEXT_H__

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * A TextEdit Text widget is a text field that is displayed as part of the on-screen keyboard.
 *
 * @see nk_console_textedit()
 * @internal
 */
NK_API nk_console* nk_console_textedit_text(nk_console* textedit);
NK_API struct nk_rect nk_console_textedit_text_render(nk_console* widget);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TEXTEDIT_TEXT_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_TEXTEDIT_TEXT_IMPLEMENTATION_ONCE
#define NK_CONSOLE_TEXTEDIT_TEXT_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Get the byte offset of the given glyph index in UTF-8 text.
 *
 * Malformed bytes count as one glyph each, so every byte belongs to a glyph.
 *
 * @param text The UTF-8 text.
 * @param len The length of the text in bytes.
 * @param glyph The glyph index. Use a negative value to count all glyphs.
 * @param glyphs Optional output for the number of glyphs walked.
 *
 * @return The byte offset of the glyph, clamped to the length of the text.
 * @internal
 */
static int nk_console_textedit_text_offset(const char* text, int len, int glyph, int* glyphs) {
    int offset = 0;
    int count = 0;
    while (offset < len && (glyph < 0 || count < glyph)) {
        nk_rune unicode;
        int glyph_len = nk_utf_decode(text + offset, &unicode, len - offset);
        offset += glyph_len > 0 ? glyph_len : 1;
        count++;
    }
    if (glyphs != NULL) {
        *glyphs = count;
    }
    return offset;
}

/**
 * Display a masked text edit, with one '*' per UTF-8 glyph, and apply its edits to the real buffer.
 *
 * The cursor and selection from before and after the edit locate where the edit happened, so text
 * is inserted and deleted at the right position in the real buffer, in whole glyphs.
 *
 * @internal
 */
static void nk_console_textedit_text_edit_masked(struct nk_context* ctx, nk_console_textedit_data* data) {
    if (data->buffer == NULL || data->buffer_size <= 0) {
        return;
    }

    int real_len = nk_strlen(data->buffer);
    if (real_len > data->buffer_size - 1) {
        real_len = data->buffer_size - 1;
    }
    int glyphs = 0;
    nk_console_textedit_text_offset(data->buffer, real_len, -1, &glyphs);

    // One '*' per glyph always fits in the buffer size. Zero it, with an extra byte, so text
    // measuring that reads up to a null terminator stays in bounds while Nuklear edits it.
    char* mask = (char*)nk_console_malloc(nk_handle_id(0), NULL, (nk_size)data->buffer_size + 1);
    if (mask == NULL) {
        return;
    }
    nk_zero(mask, (nk_size)data->buffer_size + 1);
    for (int i = 0; i < glyphs; i++) {
        mask[i] = '*';
    }
    mask[glyphs] = '\0';

    // The edit can't start after the cursor, or the selection when there is one.
    int edit_start = glyphs;
    struct nk_window* win = ctx->current;
    if (win != NULL && win->edit.active && win->edit.name == win->edit.seq) {
        if (win->edit.sel_start != win->edit.sel_end) {
            edit_start = NK_MIN(win->edit.sel_start, win->edit.sel_end);
        }
        else {
            edit_start = win->edit.cursor;
        }
        edit_start = NK_CLAMP(0, edit_start, glyphs);
    }

    // Keep room for the extra bytes of multi-byte glyphs, so the real buffer can't overflow.
    nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, mask, data->buffer_size - (real_len - glyphs), data->filter);

    int mask_len = nk_strlen(mask);
    nk_bool changed = mask_len != glyphs;
    for (int i = 0; !changed && i < mask_len; i++) {
        changed = mask[i] != '*';
    }

    if (changed) {
        int mask_glyphs = 0;
        nk_console_textedit_text_offset(mask, mask_len, -1, &mask_glyphs);

        // Everything after the cursor is unchanged, as is everything before the edit started.
        int suffix = mask_glyphs - NK_CLAMP(0, ctx->text_edit.cursor, mask_glyphs);
        suffix = NK_MIN(suffix, glyphs);
        while (suffix > 0 && mask[mask_len - suffix] != '*') {
            suffix--;
        }
        int prefix = NK_MIN(edit_start, glyphs - suffix);
        prefix = NK_MIN(prefix, mask_glyphs - suffix);
        for (int i = 0; i < prefix; i++) {
            if (mask[i] != '*') {
                prefix = i;
                break;
            }
        }

        // The unchanged '*' are single bytes, so the inserted text sits between them.
        const char* insert = mask + prefix;
        int insert_len = mask_len - suffix - prefix;
        int delete_start = nk_console_textedit_text_offset(data->buffer, real_len, prefix, NULL);
        int delete_end = nk_console_textedit_text_offset(data->buffer, real_len, glyphs - suffix, NULL);
        int tail_len = real_len - delete_end;

        // Only insert whole glyphs that fit.
        if (insert_len > data->buffer_size - 1 - delete_start - tail_len) {
            insert_len = data->buffer_size - 1 - delete_start - tail_len;
            while (insert_len > 0 && (insert[insert_len] & 0xC0) == 0x80) {
                insert_len--;
            }
        }

        // Move the unchanged tail, then copy in the inserted text.
        int tail_start = delete_start + insert_len;
        if (tail_start < delete_end) {
            for (int i = 0; i < tail_len; i++) {
                data->buffer[tail_start + i] = data->buffer[delete_end + i];
            }
        }
        else {
            for (int i = tail_len - 1; i >= 0; i--) {
                data->buffer[tail_start + i] = data->buffer[delete_end + i];
            }
        }
        for (int i = 0; i < insert_len; i++) {
            data->buffer[delete_start + i] = insert[i];
        }
        data->buffer[tail_start + tail_len] = '\0';
        data->changed = nk_true;
    }

    nk_console_mfree(nk_handle_id(0), mask);
}

NK_API struct nk_rect nk_console_textedit_text_render(nk_console* widget) {
    NK_ASSERT(widget != NULL);

    struct nk_rect widget_bounds = nk_layout_widget_bounds(widget->ctx);

    nk_console_layout_widget(widget);

    nk_console* textedit = widget->parent;
    nk_console_textedit_data* data = (nk_console_textedit_data*)textedit->data;
    nk_console_top_data* top_data = (nk_console_top_data*)nk_console_get_top(widget)->data;

    // Process checking the up/down switching in widgets before processing showing the widget itself
    if (nk_console_is_active_widget(widget) && top_data->input_processed == nk_false) {
        // Allow using ENTER to go back
        if (nk_console_button_pushed(widget, NK_GAMEPAD_BUTTON_A)) {
            top_data->input_processed = nk_true;
            nk_console_textedit_button_back_click(widget, NULL);
            return nk_rect(0, 0, 0, 0);
        }
        // Allow changing up/down only if they're not pressing backspace
        else if (!nk_input_is_key_pressed(&widget->ctx->input, NK_KEY_BACKSPACE)) {
            nk_console_check_up_down(widget);
        }

        // Display the tooltip for the textedit.
        nk_console_check_tooltip(textedit);
    }

    if (widget->disabled) {
        nk_widget_disable_begin(widget->ctx);
    }

    // Display the text edit
    if (nk_console_is_active_widget(widget)) {
        nk_edit_focus(widget->ctx, NK_EDIT_FIELD);
    }
    else {
        nk_edit_unfocus(widget->ctx);
    }

    // Allow for masking the textedit.
    if (data->masked) {
        nk_console_textedit_text_edit_masked(widget->ctx, data);
    }
    else {
        int buffer_strlen = data->buffer != NULL ? (int)nk_strlen(data->buffer) : 0;
        nk_edit_string_zero_terminated(widget->ctx, NK_EDIT_FIELD, data->buffer, data->buffer_size, data->filter);
        if (buffer_strlen != (int)nk_strlen(data->buffer)) {
            data->changed = nk_true;
        }
    }

    if (widget->disabled) {
        nk_widget_disable_end(widget->ctx);
    }

    return widget_bounds;
}

NK_API nk_console* nk_console_textedit_text(nk_console* parent) {
    nk_console* textedit_text = nk_console_label(parent, parent->label);
    textedit_text->type = NK_CONSOLE_TEXTEDIT_TEXT;
    textedit_text->parent = parent;
    textedit_text->alignment = NK_TEXT_LEFT;
    textedit_text->columns = 1;
    textedit_text->selectable = nk_true;
    textedit_text->render = nk_console_textedit_text_render;
    return textedit_text;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_TEXTEDIT_TEXT_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
