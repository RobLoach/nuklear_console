/**
 * @file Provides the ability to stub out the use of nuklear_gamepad.
 *
 * Usually in order to use nuklear_console, you would include nuklear_gamepad.h prior to using nuklear_console.h. This stub allows skipping nuklear_gamepad, disabling gamepad support entirely.
 *
 * While using nuklear_gamepad is recommended, you are able to use nuklear_console without it.
 *
 * @see https://github.com/robloach/nuklear_gamepad
 */

#ifndef NK_CONSOLE_GAMEPAD_STUB_H__
#define NK_CONSOLE_GAMEPAD_STUB_H__

// Auto-detect nuklear_gamepad is included. Use it if it is, place a stub otherwise.
#if defined(NUKLEAR_GAMEPAD_H__) && !defined(NK_CONSOLE_NO_GAMEPAD) && !defined(NK_CONSOLE_GAMEPAD)
#define NK_CONSOLE_GAMEPAD
#endif

#ifndef NK_CONSOLE_GAMEPAD
/**
 * Stubbed nk_gamepads for mocked usage.
 */
typedef struct nk_gamepads {
    int num;
} nk_gamepads;

enum nk_gamepad_button {
    NK_GAMEPAD_BUTTON_INVALID = -1,
    NK_GAMEPAD_BUTTON_FIRST = 0,
    NK_GAMEPAD_BUTTON_UP = 0,
    NK_GAMEPAD_BUTTON_DOWN = 1,
    NK_GAMEPAD_BUTTON_LEFT = 2,
    NK_GAMEPAD_BUTTON_RIGHT = 3,
    NK_GAMEPAD_BUTTON_A = 4,
    NK_GAMEPAD_BUTTON_B = 5,
    NK_GAMEPAD_BUTTON_X = 6,
    NK_GAMEPAD_BUTTON_Y = 7,
    NK_GAMEPAD_BUTTON_LB = 8,
    NK_GAMEPAD_BUTTON_RB = 9,
    NK_GAMEPAD_BUTTON_BACK = 10,
    NK_GAMEPAD_BUTTON_START = 11,
    NK_GAMEPAD_BUTTON_GUIDE = 12,
    NK_GAMEPAD_BUTTON_LAST = 13
};

enum nk_gamepad_axis {
    NK_GAMEPAD_AXIS_INVALID = -1,
    NK_GAMEPAD_AXIS_FIRST = 0,
    NK_GAMEPAD_AXIS_LEFT_X = 0,
    NK_GAMEPAD_AXIS_LEFT_Y = 1,
    NK_GAMEPAD_AXIS_RIGHT_X = 2,
    NK_GAMEPAD_AXIS_RIGHT_Y = 3,
    NK_GAMEPAD_AXIS_LEFT_TRIGGER = 4,
    NK_GAMEPAD_AXIS_RIGHT_TRIGGER = 5,
    NK_GAMEPAD_AXIS_LAST = 6
};

#endif /* !NK_CONSOLE_GAMEPAD */

#endif /* !NK_CONSOLE_GAMEPAD_STUB_H__ */

#if defined(NK_CONSOLE_IMPLEMENTATION) && !defined(NK_CONSOLE_GAMEPAD) && !defined(NK_CONSOLE_GAMEPAD_STUB_IMPLEMENTATION_ONCE)
#define NK_CONSOLE_GAMEPAD_STUB_IMPLEMENTATION_ONCE

#if defined(__cplusplus)
extern "C" {
#endif

static float nk_gamepad_get_axis(struct nk_gamepads* g, int n, enum nk_gamepad_axis a) {
    NK_UNUSED(g);
    NK_UNUSED(n);
    NK_UNUSED(a);
    return 0.0f;
}

static nk_bool nk_gamepad_is_button_released(struct nk_gamepads* g, int n, enum nk_gamepad_button b) {
    NK_UNUSED(g);
    NK_UNUSED(n);
    NK_UNUSED(b);
    return nk_false;
}

static nk_bool nk_gamepad_is_button_down(struct nk_gamepads* g, int n, enum nk_gamepad_button b) {
    NK_UNUSED(g);
    NK_UNUSED(n);
    NK_UNUSED(b);
    return nk_false;
}

static nk_bool nk_gamepad_init(struct nk_gamepads* gamepads, struct nk_context* ctx, void* user_data) {
    NK_UNUSED(gamepads);
    NK_UNUSED(ctx);
    NK_UNUSED(user_data);
    return nk_true;
}

static void nk_gamepad_free(struct nk_gamepads* gamepads) {
    NK_UNUSED(gamepads);
}

static void nk_gamepad_update(struct nk_gamepads* gamepads) {
    NK_UNUSED(gamepads);
}

#if defined(__cplusplus)
}
#endif

#endif
