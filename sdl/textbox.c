#include "main.h"
#include "sprite.h"
#include "textbox.h"
#include "gfftypes.h"
#include "font.h"
#include "utils.h"
#include "gff.h"
#include "settings.h"

int textbox_handle_keydown(textbox_t *tb, SDL_Keysym ks) {
    if (!tb) { return 0; }
    if (!tb->in_focus) { return 0; }
    if (ks.sym == SDLK_ESCAPE) { tb->in_focus = 0; }
    size_t slen = strlen(tb->text);

    if ((ks.sym >= 'a' && ks.sym <= 'z') || (ks.sym >= '0' && ks.sym <= '9')) {
        if (slen < tb->text_len - 1) {
            tb->text[slen + 1] = '\0';
            while (slen > tb->text_pos) {
                tb->text[slen] = tb->text[slen - 1];
                slen--;
            }
            if (ks.sym >= 'a' && ks.sym <= 'z' && (ks.mod & KMOD_SHIFT)) {
                tb->text[tb->text_pos] = ks.sym - 32;
            } else {
                tb->text[tb->text_pos] = ks.sym;
            }
            //tb->text[tb->text_pos + 1] = '\0';
            tb->text_pos++;
        }
    }
    if (ks.sym == SDLK_BACKSPACE && tb->text_pos > 0) {
        for (size_t i = tb->text_pos - 1; i < slen; i++) {
            tb->text[i] = tb->text[i + 1];
        }
        tb->text_pos--;
    }
    if (ks.sym == SDLK_DELETE && tb->text_pos >= 0) {
        for (size_t i = tb->text_pos; i < slen; i++) {
            tb->text[i] = tb->text[i + 1];
        }
    }
    if (ks.sym == SDLK_LEFT && tb->text_pos > 0) {
        tb->text_pos--;
    }
    if (ks.sym == SDLK_RIGHT && tb->text_pos < strlen(tb->text)) {
        tb->text_pos++;
    }
    return 1;
}

int textbox_handle_keyup(textbox_t *tb, SDL_Keysym ks) {
    if (!tb) { return 0; }
    if (!tb->in_focus) { return 0; }
    if (ks.sym == SDLK_RETURN) { tb->in_focus = 0; }
    return 1;
}

static uint16_t calc_offset(textbox_t *tb) {
    if (!tb) { return 0; }
    uint16_t offset = 0;

    for (size_t i = 0; i < tb->text_pos && tb->text[i]; i++) {
        offset += font_char_width(FONT_GREYLIGHT, tb->text[i]);
    }

    return offset;
}

void sol_textbox_render(textbox_t *tb) {
    if (!tb) { return; }
    print_line_len(main_get_rend(), FONT_GREYLIGHT, tb->text, tb->xpos, tb->ypos, tb->text_len);

    if (tb->in_focus) {
        tb->cursor_countdown--;
        if (tb->cursor_countdown >= 30) {
            const uint16_t cx = tb->xpos - (2 * settings_zoom()) + calc_offset(tb);
            const uint16_t cy = tb->ypos + (1 * settings_zoom());
            sol_sprite_set_location(tb->text_cursor, cx, cy);
            sol_sprite_render(tb->text_cursor);
        }
        if (tb->cursor_countdown <= 0) {
            tb->cursor_countdown = 60;
        }
    }
}
