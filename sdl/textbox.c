#include "main.h"
#include "sprite.h"
#include "textbox.h"
#include "../src/gff.h"
#include "../src/gfftypes.h"
#include "font.h"

textbox_t* textbox_create(const size_t len, const uint16_t _xpos, const uint16_t _ypos) {
    gff_palette_t* pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    textbox_t *ret = calloc(1, sizeof(textbox_t));

    ret->xpos = _xpos * main_get_zoom();
    ret->ypos = _ypos * main_get_zoom();
    ret->text = calloc(1, len);
    ret->text_cursor = sprite_new(main_get_rend(), pal, ret->xpos, ret->ypos, // Blinking text cursor (for the player's name)
        main_get_zoom(), RESOURCE_GFF_INDEX, GFF_ICON, 100);

    return ret;
}

void textbox_set_text(textbox_t *tb, char *text) {
    size_t len = strlen(text);

    if (len > tb->text_len) {
        tb->text = realloc(tb->text, len + 1);
    }

    strcpy(tb->text, text);
}

void textbox_render(textbox_t *tb) {
    if (!tb) { return; }
    print_line_len(main_get_rend(), FONT_BLACKDARK, tb->text, tb->xpos, tb->ypos, tb->text_len);
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

