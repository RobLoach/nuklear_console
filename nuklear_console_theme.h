#ifndef NK_CONSOLE_THEME_H__
#define NK_CONSOLE_THEME_H__

#ifndef NUKLEAR_TINTED_H
#error "nuklear_console_theme.h requires nuklear_tinted.h to be included first"
#endif

#ifndef NK_CONSOLE_THEME_DEFAULT_LABEL
/** The label for a NULL scheme, which applies Nuklear's default style. */
#define NK_CONSOLE_THEME_DEFAULT_LABEL "Default"
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Add a combobox that applies the chosen tinted-c scheme to the context's style.
 *
 * The chosen scheme applies right away, and again whenever it changes. Handlers added to the returned widget for
 * NK_CONSOLE_EVENT_CHANGED run after the scheme applies, so they can adjust the style further.
 *
 * @param parent The parent widget.
 * @param label The combobox label.
 * @param schemes The schemes to choose from, by their names. A NULL entry applies Nuklear's default style, labelled
 * NK_CONSOLE_THEME_DEFAULT_LABEL. The array must outlive the widget.
 * @param count The number of schemes.
 * @param selected The index of the chosen scheme, which is kept in range.
 *
 * @return The new theme widget.
 */
NK_API nk_console* nk_console_theme(nk_console* parent, const char* label, const tinted_scheme* const* schemes, int count, int* selected);
/** Internal: apply the theme widget's chosen scheme. */
NK_API void nk_console_theme_changed(nk_console* theme, void* user_data);

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_THEME_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_THEME_IMPLEMENTATION_ONCE
#define NK_CONSOLE_THEME_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API void nk_console_theme_changed(nk_console* theme, void* user_data) {
    const tinted_scheme* const* schemes = (const tinted_scheme* const*)user_data;

    // The combobox shows the chosen option's label, so match it to find the scheme. The first child is Back.
    for (size_t i = 1; i < cvector_size(theme->children); i++) {
        if (theme->children[i]->label == theme->label) {
            if (schemes[i - 1] == NULL) {
                nk_style_default(theme->ctx);
            }
            else {
                nuklear_tinted_set_scheme(theme->ctx, schemes[i - 1]);
            }
            return;
        }
    }
}

NK_API nk_console* nk_console_theme(nk_console* parent, const char* label, const tinted_scheme* const* schemes, int count, int* selected) {
    // Creating the combobox without items resets the selection, so keep it to restore.
    int initial = selected != NULL ? *selected : 0;
    nk_console* theme = nk_console_combobox(parent, label, NULL, '|', selected);
    if (theme == NULL) {
        return NULL;
    }

    for (int i = 0; i < count; i++) {
        nk_console_button_onclick(theme, schemes[i] != NULL ? schemes[i]->name : NK_CONSOLE_THEME_DEFAULT_LABEL, &nk_console_combobox_button_click);
    }

    if (selected != NULL && count > 0) {
        *selected = initial < 0 ? 0 : (initial >= count ? count - 1 : initial);
        theme->label = theme->children[*selected + 1]->label;
        theme->label_length = theme->children[*selected + 1]->label_length;
    }

    nk_console_add_event_handler(theme, NK_CONSOLE_EVENT_CHANGED, &nk_console_theme_changed, (void*)schemes, NULL);
    nk_console_theme_changed(theme, (void*)schemes);
    return theme;
}

#if defined(__cplusplus)
}
#endif

#endif // NK_CONSOLE_THEME_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
