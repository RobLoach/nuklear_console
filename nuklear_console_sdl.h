#ifndef NK_CONSOLE_SDL_H__
#define NK_CONSOLE_SDL_H__

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Call once per frame after rendering to sync SDL text input state with the
 * active console widget. Starts text input when a textedit field is active,
 * stops it otherwise.
 *
 * Compatible with both SDL2 and SDL3. For SDL2, window may be NULL.
 */
NK_INTERN void nk_console_sdl_update_text_input(nk_console* console, SDL_Window* window) {
    nk_console* top = nk_console_get_top(console);
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    nk_console* active = (top_data->active_parent != NULL)
        ? top_data->active_parent->activeWidget
        : NULL;
    nk_bool wants_text = (active != NULL && active->type == NK_CONSOLE_TEXTEDIT_TEXT);
    if (wants_text) {
#if SDL_MAJOR_VERSION >= 3
        SDL_StartTextInput(window);
#else
        (void)window;
        SDL_StartTextInput();
#endif
    } else {
#if SDL_MAJOR_VERSION >= 3
        SDL_StopTextInput(window);
#else
        (void)window;
        SDL_StopTextInput();
#endif
    }
}

#if defined(__cplusplus)
}
#endif

#endif /* NK_CONSOLE_SDL_H__ */
