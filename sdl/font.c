#include "font.h"
#include "../src/dsl.h"

#define MAX_CHARS (128)

static SDL_Texture *font_table[NUM_FONTS][MAX_CHARS];
static SDL_Rect     font_loc[NUM_FONTS][MAX_CHARS];

static void create_font(SDL_Renderer *renderer, const uint32_t idx, const uint32_t fg_color, const uint32_t bg_color) {
    char *data = NULL;
    SDL_Surface *surface = NULL;
    ds_char_t *ds_char = NULL;
    gff_chunk_header_t chunk = gff_find_chunk_header(RESOURCE_GFF_INDEX, GFF_FONT, 100);
    ds_font_t *dsfont = malloc(chunk.length);

    if (!dsfont) {
        error ("unable to malloc for font!\n");
        exit(1);
    }

    gff_read_chunk(RESOURCE_GFF_INDEX, &chunk, dsfont, chunk.length);
    for (int c = 0; c < MAX_CHARS; c++ ) {
        ds_char = (ds_char_t*)(((uint8_t*)dsfont) + dsfont->char_offset[c]);
        data = (char*)create_font_rgba(RESOURCE_GFF_INDEX, c, fg_color, bg_color);
        font_loc[idx][c].w = ds_char->width;
        font_loc[idx][c].h = dsfont->height;
        surface = SDL_CreateRGBSurfaceFrom(data, font_loc[idx][c].w, font_loc[idx][c].h, 32, 4*font_loc[idx][c].w,
                0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        font_table[idx][c] = SDL_CreateTextureFromSurface(renderer, surface);
        font_loc[idx][c].w *= 2;
        font_loc[idx][c].h *= 2;
        SDL_FreeSurface(surface);
        free(data);
    }
    free(dsfont);
}

uint32_t font_pixel_width(font_t font, const char *text, const uint32_t len) {
    uint32_t sum = 0;
    size_t c;

    if (text == NULL) { return sum; }

    for (int i = 0; text[i] && i < len; i++) {
        c = text[i];
        sum += font_loc[font][c].w;
        //debug("Need to print '%c'\n", i);
    }

    return sum;
}

void print_line_len(SDL_Renderer *renderer, font_t font, const char *text, size_t x, size_t y, const uint32_t len) {
    size_t c;
    if (text == NULL) { return; }
    for (int i = 0; text[i] && i < len; i++) {
        c = text[i];
        font_loc[font][c].x = x;
        font_loc[font][c].y = y;
        SDL_RenderCopy(renderer, font_table[font][c], NULL, &font_loc[font][c]);
        x += font_loc[font][c].w;
        //debug("Need to print '%c'\n", i);
    }
}

void font_init(SDL_Renderer *renderer) {
    memset(font_table, 0x0, NUM_FONTS * sizeof(SDL_Texture*) * MAX_CHARS);
    memset(font_loc, 0x0, NUM_FONTS * sizeof(SDL_Rect) * MAX_CHARS);
    create_font(renderer, FONT_YELLOW, 0xFFFF18FF, 0x101038FF);
    create_font(renderer, FONT_GREY, 0x8A8AA2FF, 0x101038FF);
    create_font(renderer, FONT_GREYLIGHT, 0xD9D9E5FF, 0x11112BFF); // Really 0xD7D7E3FF and 0x202038FF
    create_font(renderer, FONT_BLACK, 0x101038FF, 0xA6A6BEFF);
    create_font(renderer, FONT_BLACKDARK, 0x000000FF, 0xA67171FF);
    create_font(renderer, FONT_RED, 0xD72128FF, 0xA6A6BEFF);
    create_font(renderer, FONT_REDDARK, 0xD72128FF, 0x000000FF);
    create_font(renderer, FONT_BLUE, 0x287DC7FF, 0x101038FF);
}

void font_free(SDL_Renderer *renderer) {
    for (int i = 0; i < MAX_CHARS; i++) {
        SDL_DestroyTexture(font_table[0][i]);
    }
}
