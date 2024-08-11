#ifndef NK_CONSOLE_HPP__
#define NK_CONSOLE_HPP__

#include <utility>

template <typename T>
void nk_console_event_handler_call(nk_console* widget, void* user_data) {
    T* t = reinterpret_cast<T*>(user_data);
    (*t)(widget);
}

template <typename T>
void nk_console_event_handler_destroy(nk_console*, void* user_data) {
    T* t = reinterpret_cast<T*>(user_data);
    t->~T();
    nk_console_mfree(nk_handle_id(0), user_data);
}

template <typename T>
void nk_console_set_event_handler(nk_console* widget, nk_console_event_type type, T&& t) {
    void* memory = nk_console_malloc(nk_handle_id(0), NULL, sizeof(T));
    T* user_data = new (memory) T(std::move(t));
    nk_console_add_event_handler(widget, type, &nk_console_event_handler_call<T>, user_data, &nk_console_event_handler_destroy<T>);
}

template <typename T>
void nk_console_set_onchange_handler(nk_console* widget, T&& t) {
    nk_console_set_event_handler(widget, NK_CONSOLE_EVENT_CHANGED, std::move(t));
}

template <typename T>
void nk_console_button_set_onclick_handler(nk_console* button, T&& t) {
    nk_console_set_event_handler(button, NK_CONSOLE_EVENT_CLICKED, std::move(t));
}

template <typename T>
nk_console* nk_console_button_onclick_handler(nk_console* parent, const char* text, T&& t) {
    nk_console* button = nk_console_button(parent, text);
    nk_console_button_set_onclick_handler(button, std::move(t));
    return button;
}

#endif  // NK_CONSOLE_HPP__
