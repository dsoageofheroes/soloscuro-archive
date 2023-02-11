#include "font.h"
#include "main.h"
#include "gpl.h"
#include "settings.h"
#include <SDL2/SDL_ttf.h>

#define MAX_CHARS (128)

static SDL_Texture *font_table[NUM_FONTS][MAX_CHARS];
static SDL_Rect     font_loc[NUM_FONTS][MAX_CHARS];
static TTF_Font    *font = NULL;

static void create_font(SDL_Renderer *renderer, const uint32_t idx, const uint32_t fg_color, const uint32_t bg_color) {
    char *data = NULL;
    SDL_Surface *surface = NULL;
    ds_char_t *ds_char = NULL;
    gff_chunk_header_t chunk;
    ds_font_t *dsfont = NULL;
    int resource_gff = -1;

    switch(gff_get_game_type()) {
        case DARKSUN_1:
            resource_gff = RESOURCE_GFF_INDEX;
            break;
        case DARKSUN_2:
            resource_gff = RESFLOP_GFF_INDEX;
            break;
        case DARKSUN_ONLINE:
            resource_gff = RESFLOP_GFF_INDEX;
            break;
        default:
            error("Unknown Darksun gffs. Did you set the correct path?");
            exit(1);
            break;
    }
    chunk = gff_find_chunk_header(resource_gff, GFF_FONT, 100);
    dsfont = (ds_font_t*) malloc(chunk.length);

    if (!dsfont) {
        error ("unable to malloc for font!\n");
        exit(1);
    }

    gff_read_chunk(resource_gff, &chunk, dsfont, chunk.length);
    for (int c = 0; c < MAX_CHARS; c++ ) {
        ds_char = (ds_char_t*)(((uint8_t*)dsfont) + dsfont->char_offset[c]);
        data = (char*)gff_create_font_rgba(resource_gff, c, fg_color, bg_color);
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

extern sol_status_t sol_font_pixel_width(sol_font_t font, const char *text, const uint32_t len, uint32_t *width) {
    uint32_t sum = 0;
    size_t c;

    if (text == NULL || width == NULL) { return SOL_NULL_ARGUMENT; }

    for (size_t i = 0; text[i] && i < len; i++) {
        c = text[i];
        sum += font_loc[font][c].w;
        //debug("Need to print '%c'\n", i);
    }

    *width = sum;
    return SOL_SUCCESS;
}

uint16_t font_char_width(sol_font_t font, const int c) {
    return font_loc[font][c].w;
}

extern sol_status_t sol_font_pixel_height(sol_font_t font, uint32_t *height) {
    if (!height) { return SOL_NULL_ARGUMENT; }
    *height = font_loc[font]['T'].h; // font height is the same for all characters
    return SOL_SUCCESS;
}

void font_render_ttf(const char *msg, uint16_t x, uint16_t y, uint32_t color) {
    SDL_Color text_color = {
        (color >> 24) & 0xFF,
        (color >> 16) & 0xFF,
        (color >> 8) & 0xFF,
        (color >> 0) & 0xFF
    };
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;

    // TODO: Consider creating font maps, instead of create/destory textures each time.
    // Fast, probably okay for create/destroy
    SDL_Surface *textSurface = TTF_RenderUTF8_Solid( font, msg, text_color );
    // Slow, for labels and such, blending has alpha.
    //SDL_Surface *textSurface = TTF_RenderUTF8_Blended( font, msg, text_color );
    offset.w = textSurface->w;
    offset.h = textSurface->h;
    SDL_Texture *tex = SDL_CreateTextureFromSurface( main_get_rend(), textSurface );
    SDL_RenderCopy(main_get_rend(), tex, NULL, &offset); //(sprite->loc + sprite->pos));
    SDL_FreeSurface( textSurface );
    SDL_DestroyTexture( tex );
}

static sol_status_t print_line_len(SDL_Renderer *renderer, sol_font_t font, const char *text, size_t x, size_t y, const uint32_t len) {
    size_t c;
    if (text == NULL) { return SOL_NULL_ARGUMENT; }

    for (uint32_t i = 0; text[i] && i < len; i++) {
        c = text[i];
        font_loc[font][c].x = x;
        font_loc[font][c].y = y;
        SDL_RenderCopy(renderer, font_table[font][c], NULL, &font_loc[font][c]);
        x += font_loc[font][c].w;
        //debug("Need to print '%c'\n", i);
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_print_line_len(const sol_font_t font, const char *text, size_t x, size_t y, const uint32_t len) {
    return print_line_len(main_get_rend(), font, text, x, y, len);
}

extern void font_init(SDL_Renderer *renderer) {
    memset(font_table, 0x0, NUM_FONTS * sizeof(SDL_Texture*) * MAX_CHARS);
    memset(font_loc, 0x0, NUM_FONTS * sizeof(SDL_Rect) * MAX_CHARS);
    create_font(renderer, FONT_YELLOW, 0xFFFF18FF, 0x101038FF);
    create_font(renderer, FONT_YELLOW_BRIGHT, 0xFFFF28FF, 0xFFFFFFFF);
    create_font(renderer, FONT_GREY, 0x8A8AA2FF, 0x101038FF);
    create_font(renderer, FONT_GREYLIGHT, 0xD9D9E5FF, 0x11112BFF); // Really 0xD7D7E3FF and 0x202038FF
    create_font(renderer, FONT_BLACK, 0x101038FF, 0xA6A6BEFF);
    create_font(renderer, FONT_BLACKDARK, 0x000000FF, 0xA67171FF);
    create_font(renderer, FONT_RED, 0xD72128FF, 0xA6A6BEFF);
    create_font(renderer, FONT_REDDARK, 0xD72128FF, 0x000000FF);
    create_font(renderer, FONT_BLUE, 0x287DC7FF, 0x101038FF);
    TTF_Init();
    font = TTF_OpenFont( "DarkSun.ttf", 8 * settings_zoom() );
}

static sol_status_t font_render_center(SDL_Renderer *rend, sol_font_t font, const char *str, const SDL_Rect loc) {
    int len = strlen(str);
    int pixel_width = 0;
    sol_status_t status;

    if ((status = sol_font_pixel_width(font, str, len, &pixel_width)) != SOL_SUCCESS) {
        return status;
    }

    int offset = (loc.w / 2) - (pixel_width / 2);
    return print_line_len(rend, font, str, loc.x + offset, loc.y, len);
}

extern sol_status_t sol_font_render_center(sol_font_t font, const char *str, const uint16_t x, const uint16_t y, const uint16_t w) {
    SDL_Rect loc;
    loc.x = x; loc.y = y; loc.w = w; loc.h = 10;
    return font_render_center(main_get_rend(), font, str, loc);
}

extern sol_status_t sol_font_free() {
    for (int i = 0; i < MAX_CHARS; i++) {
        SDL_DestroyTexture(font_table[0][i]);
    }

    //Free global font
    TTF_CloseFont(font);
    TTF_Quit();
    font = NULL;

    return SOL_SUCCESS;
}
