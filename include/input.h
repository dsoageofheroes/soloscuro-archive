#ifndef SOL_INPUT_H
#define SOL_INPUT_H

typedef enum sol_key_e {
    SOLK_RETURN = 1,
    SOLK_ESCAPE, SOLK_BACKSPACE, SOLK_TAB, SOLK_SPACE, SOLK_EXCLAIM, SOLK_QUOTEDBL,
    SOLK_HASH, SOLK_PERCENT, SOLK_DOLLAR, SOLK_AMPERSAND, SOLK_QUOTE, SOLK_LEFTPAREN,
    SOLK_RIGHTPAREN, SOLK_ASTERISK, SOLK_PLUS, SOLK_COMMA, SOLK_MINUS, SOLK_PERIOD, SOLK_SLASH,
    SOLK_0, SOLK_1, SOLK_2, SOLK_3, SOLK_4, SOLK_5, SOLK_6, SOLK_7, SOLK_8, SOLK_9,
    SOLK_COLON, SOLK_SEMICOLON, SOLK_LESS, SOLK_EQUALS, SOLK_GREATER, SOLK_QUESTION, SOLK_AT,
    SOLK_LEFTBRACKET, SOLK_BACKSLASH, SOLK_RIGHTBRACKET, SOLK_CARET, SOLK_UNDERSCORE, SOLK_BACKQUOTE,
    SOLK_a, SOLK_b, SOLK_c, SOLK_d, SOLK_e, SOLK_f, SOLK_g, SOLK_h, SOLK_i, SOLK_j, SOLK_k,
    SOLK_l, SOLK_m, SOLK_n, SOLK_o, SOLK_p, SOLK_q, SOLK_r, SOLK_s, SOLK_t, SOLK_u, SOLK_v,
    SOLK_w, SOLK_x, SOLK_y, SOLK_z,
    SOLK_CAPSLOCK, SOLK_F1, SOLK_F2, SOLK_F3, SOLK_F4, SOLK_F5, SOLK_F6, SOLK_F7, SOLK_F8, SOLK_F9,
    SOLK_F10, SOLK_F11, SOLK_F12, SOLK_PRINTSCREEN, SOLK_SCROLLLOCK, SOLK_PAUSE, SOLK_INSERT,
    SOLK_HOME, SOLK_PAGEUP, SOLK_DELETE, SOLK_END, SOLK_PAGEDOWN, SOLK_RIGHT, SOLK_LEFT, SOLK_DOWN,
    SOLK_UP, SOLK_NUMLOCKCLEAR, SOLK_KP_DIVIDE, SOLK_KP_MULTIPLY, SOLK_KP_MINUS, SOLK_KP_PLUS,
    SOLK_KP_ENTER, SOLK_KP_1, SOLK_KP_2, SOLK_KP_3, SOLK_KP_4, SOLK_KP_5, SOLK_KP_6, SOLK_KP_7,
    SOLK_KP_8, SOLK_KP_9, SOLK_KP_0, SOLK_KP_PERIOD 
} sol_key_e;

extern void sol_key_down(const sol_key_e key);
extern void sol_key_up(const sol_key_e key);
extern void sol_input_tick();
extern int sol_in_debug_mode();

#endif
