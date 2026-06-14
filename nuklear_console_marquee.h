#ifndef NK_CONSOLE_MARQUEE_H__
#define NK_CONSOLE_MARQUEE_H__

#ifndef NK_CONSOLE_MARQUEE_SCROLL_SPEED
#define NK_CONSOLE_MARQUEE_SCROLL_SPEED 60.0f
#endif
#ifndef NK_CONSOLE_MARQUEE_SCROLL_PAUSE
#define NK_CONSOLE_MARQUEE_SCROLL_PAUSE 1.5f
#endif

#endif // NK_CONSOLE_MARQUEE_H__

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_HEADER_ONLY)
#ifndef NK_CONSOLE_MARQUEE_IMPLEMENTATION_ONCE
#define NK_CONSOLE_MARQUEE_IMPLEMENTATION_ONCE

/**
 * Advance a marquee scroll offset and return a pointer to the visible slice of text.
 * Returns `text` unchanged when the text fits or delta time is unavailable.
 * The caller owns `buf` (minimum `buf_size` bytes).
 */
static const char* nk_console_marquee_slice(
    struct nk_context* ctx,
    const char* text, int text_len,
    float full_text_width, float avail_width,
    float speed, float pause,
    float* scroll_x,
    char* buf, int buf_size)
{
    if (full_text_width <= avail_width || ctx->delta_time_seconds <= 0) {
        return text;
    }
    float pause_pixels = pause * speed;
    float total_cycle = full_text_width + pause_pixels;
    *scroll_x += ctx->delta_time_seconds * speed;
    if (*scroll_x > total_cycle) {
        *scroll_x -= total_cycle;
    }
    float offset = *scroll_x - pause_pixels;
    if (offset <= 0.0f) {
        return text;
    }
    int start = 0;
    for (int i = 1; i <= text_len; i++) {
        float w = ctx->style.font->width(ctx->style.font->userdata, ctx->style.font->height, text, i);
        if (w >= offset) { start = i - 1; break; }
        if (i == text_len) { start = text_len; }
    }
    int copy_len = text_len - start;
    if (copy_len >= buf_size) {
        copy_len = buf_size - 1;
    }
    NK_MEMCPY(buf, text + start, (nk_size)copy_len);
    buf[copy_len] = '\0';
    return buf;
}

/**
 * Render a single-line marquee tooltip of `tooltip_width` at the current mock mouse position.
 */
static void nk_console_marquee_tooltip_render(
    struct nk_context* ctx,
    const char* text, int text_len,
    float full_text_width,
    float tooltip_width, float text_height,
    float speed, float pause,
    float* scroll_x)
{
    float avail_width = tooltip_width - ctx->style.window.padding.x * 2.0f;
    char display_buf[256];
    const char* display_text = nk_console_marquee_slice(ctx, text, text_len,
        full_text_width, avail_width, speed, pause, scroll_x, display_buf, (int)sizeof(display_buf));
    struct nk_vec2 zero;
    nk_zero_struct(zero);
    if (nk_tooltip_begin_offset(ctx, tooltip_width, NK_TOP_LEFT, zero)) {
        nk_layout_row_dynamic(ctx, text_height, 1);
        nk_label(ctx, display_text, NK_TEXT_LEFT);
        nk_tooltip_end(ctx);
    }
}

#endif // NK_CONSOLE_MARQUEE_IMPLEMENTATION_ONCE
#endif // NK_CONSOLE_IMPLEMENTATION
