#include "main.h"
#include "sprite.h"
#include "textbox.h"
#include "../src/gff.h"
#include "gfftypes.h"
#include "../src/settings.h"
#include "font.h"

textbox_t* textbox_create(const size_t len, const uint16_t _xpos, const uint16_t _ypos) {
    gff_palette_t* pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    textbox_t *ret = calloc(1, sizeof(textbox_t));

    ret->xpos = _xpos * settings_zoom();
    ret->ypos = _ypos * settings_zoom();
    ret->text = calloc(1, len);
    ret->text_len = len;
    ret->text_cursor = sprite_new(main_get_rend(), pal, ret->xpos, ret->ypos, // Blinking text cursor (for the player's name)
        settings_zoom(), RESOURCE_GFF_INDEX, GFF_ICON, 100);
    sprite_set_frame(ret->text_cursor, 1);

    return ret;
}

void textbox_set_text(textbox_t *tb, char *text) {
    size_t len = strlen(text);

    if (len > tb->text_len) {
        tb->text = realloc(tb->text, len + 1);
        tb->text_len = len;
    }

    strcpy(tb->text, text);
}

char* textbox_get_text(textbox_t *tb) {
    if (!tb) { return NULL; }
    return tb->text;
}

int textbox_is_in(textbox_t *tb, const uint16_t x, const uint16_t y) {
    if (!tb) { return 0; }
    int text_width = 8 * tb->text_len;

    if (x < tb->xpos || x > tb->xpos + (text_width * settings_zoom())) { return 0;}
    if (y < tb->ypos || y > tb->ypos + (8 * settings_zoom())) { return 0;}

    tb->cursor_pos = (x - tb->xpos) / (8 * settings_zoom());
    return 1;
}

void textbox_set_focus(textbox_t *tb, const int focus) {
    if (!tb) { return; }
    //printf("Set focus: %d\n", focus);
    tb->in_focus = focus;
}

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

void textbox_render(textbox_t *tb) {
    if (!tb) { return; }
    print_line_len(main_get_rend(), FONT_GREYLIGHT, tb->text, tb->xpos, tb->ypos, tb->text_len);

    if (tb->in_focus) {
        tb->cursor_countdown--;
        if (tb->cursor_countdown >= 30) {
            const uint16_t cx = tb->xpos - (2 * settings_zoom()) + calc_offset(tb);
            const uint16_t cy = tb->ypos + (1 * settings_zoom());
            sprite_set_location(tb->text_cursor, cx, cy);
            sprite_render(main_get_rend(), tb->text_cursor);
        }
        if (tb->cursor_countdown <= 0) {
            tb->cursor_countdown = 60;
        }
    }
}

void textbox_free(textbox_t *tb) {
    if (!tb) { return; }

    sprite_free(tb->text_cursor);

    if (tb->text) {
        free(tb->text);
        tb->text = NULL;
    }

    free(tb);
}

