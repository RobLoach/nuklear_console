#ifndef NK_CONSOLE_HPP__
#define NK_CONSOLE_HPP__

#include <utility>

template <typename T>
void nk_console_event_handler_call(nk_console* widget, nk_console_event_data data) {
    T* t = reinterpret_cast<T*>(data.user);
    (*t)(widget);
}

template <typename T>
void nk_console_event_handler_destroy(nk_console*, nk_console_event_data data) {
    T* t = reinterpret_cast<T*>(data.user);
    t->~T();
    nk_console_mfree(nk_handle_id(0), data.user);
}

template <typename T>
void nk_console_set_event_handler(nk_console* widget, nk_console_event_handler* handler, T&& t) {
    void* memory = nk_console_malloc(nk_handle_id(0), NULL, sizeof(T));
    nk_console_event_data data;
    data.user = new (memory) T(std::move(t));
    nk_console_set_event_handler(widget, handler, &nk_console_event_handler_call<T>, data, &nk_console_event_handler_destroy<T>);
}

template <typename T>
void nk_console_set_onchange_handler(nk_console* widget, T&& t) {
    nk_console_set_event_handler(widget, &widget->onchange, std::move(t));
}

template <typename T>
void nk_console_button_set_onclick_handler(nk_console* button, T&& t) {
    nk_console_button_data* data = reinterpret_cast<nk_console_button_data*>(button->data);
    nk_console_set_event_handler(button, &data->onclick, std::move(t));
}

template <typename T>
nk_console* nk_console_button_onclick_handler(nk_console* parent, const char* text, T&& t) {
    nk_console* button = nk_console_button(parent, text);
    nk_console_button_set_onclick_handler(button, std::move(t));
    return button;
}

#endif  // NK_CONSOLE_HPP__