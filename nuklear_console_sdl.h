#ifndef NK_CONSOLE_SDL_H__
#define NK_CONSOLE_SDL_H__

#ifndef SDL_MAJOR_VERSION
#error "nuklear_console_sdl.h requires SDL.h to be included first"
#endif

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
NK_API void nk_console_sdl_update_text_input(nk_console* console, SDL_Window* window);

#if defined(__cplusplus)
}
#endif

#endif /* NK_CONSOLE_SDL_H__ */

#ifdef NK_CONSOLE_IMPLEMENTATION
#ifndef NK_CONSOLE_SDL_IMPLEMENTATION_ONCE
#define NK_CONSOLE_SDL_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

NK_API void nk_console_sdl_update_text_input(nk_console* console, SDL_Window* window) {
    nk_console* top = nk_console_get_top(console);
    nk_console_top_data* top_data = (nk_console_top_data*)top->data;
    nk_console* active_parent = top_data->active_parent;
    nk_console* active = (active_parent != NULL) ? active_parent->activeWidget : NULL;

    // Text input must be active for SDL3 to deliver SDL_EVENT_TEXT_INPUT, which
    // is how printable characters reach Nuklear. Two console states need it:
    //   1. A Textedit Text widget is being edited.
    //   2. An input-capture widget (NK_CONSOLE_INPUT) is prompting and accepts a
    //      keyboard key (out_key != NULL); without text input its character
    //      capture would silently fail while special keys still work.
    nk_bool wants_text_input = (active != NULL && active->type == NK_CONSOLE_TEXTEDIT_TEXT);
    if (!wants_text_input && active_parent != NULL && active_parent->type == NK_CONSOLE_INPUT) {
        nk_console_input_data* input_data = (nk_console_input_data*)active_parent->data;
        if (input_data != NULL && input_data->out_key != NULL) {
            wants_text_input = nk_true;
        }
    }

    // Only start or stop text input on state transitions. In SDL3,
    // SDL_StartTextInput() re-runs property setup and the on-screen keyboard
    // check on every call, even when text input is already active.
#if SDL_MAJOR_VERSION >= 3
    if (wants_text_input != SDL_TextInputActive(window)) {
        if (wants_text_input) {
            SDL_StartTextInput(window);
        }
        else {
            SDL_StopTextInput(window);
        }
    }
#else
    (void)window;
    if (wants_text_input != (SDL_IsTextInputActive() == SDL_TRUE)) {
        if (wants_text_input) {
            SDL_StartTextInput();
        }
        else {
            SDL_StopTextInput();
        }
    }
#endif
}

#if defined(__cplusplus)
}
#endif

#endif /* NK_CONSOLE_SDL_IMPLEMENTATION_ONCE */
#endif /* NK_CONSOLE_IMPLEMENTATION */
