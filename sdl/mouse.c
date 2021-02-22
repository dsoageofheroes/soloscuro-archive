#include "mouse.h"
#include "../src/dsl.h"
#include "../src/gff.h"
#include "../src/gfftypes.h"

const static size_t ds1_icon_res[] = {
    6001, // regular
    6002, // No click
    6003, // melee
    6004, // no melee
    6005, // range
    6006, // no range
    6007, // talk
    6008, // no talk
    6009, // no magic
    6010, // wait
};

static SDL_Cursor **cursors;
static int num_cursors;
static ds1_item_t *item_data;
static uint16_t item_spr;
static SDL_Cursor *item_cursor;

static SDL_Cursor* create_cursor(const int gff_idx, const int type_idx, const size_t res_id) {
    SDL_Surface *surface = NULL;
    SDL_Cursor *c;
    SDL_Rect loc;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    unsigned char *data = get_frame_rgba_palette(gff_idx, type_idx, res_id, 0, pal);
    loc.w = get_frame_width(gff_idx, type_idx, res_id, 0);
    loc.h = get_frame_height(gff_idx, type_idx, res_id, 0);
    surface = SDL_CreateRGBSurfaceFrom(data, loc.w, loc.h, 32, 4*loc.w,
            0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    SDL_Surface *stretch = SDL_CreateRGBSurface(0, 2 * loc.w, 2 * loc.h, 32,
            0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    loc.x = 0;
    loc.y = 0;

    SDL_Rect stretchRect;
    stretchRect.x = 0;
    stretchRect.y = 0;
    stretchRect.w = loc.w * 2;
    stretchRect.h = loc.h * 2;

    SDL_BlitScaled( surface, &loc, stretch, &stretchRect );
    c = SDL_CreateColorCursor(stretch, 0, 0);
    SDL_FreeSurface(surface);
    SDL_FreeSurface(stretch);
    free(data);
    return c;
}

void mouse_init(SDL_Renderer *rend) {
    if (gff_get_game_type() != DARKSUN_1) {
        warn("Mouse cursor implemented in DS1 only right now. Not loading mouse cursor.");
        return;
    }
    num_cursors = sizeof(ds1_icon_res) / sizeof(size_t);
    cursors = malloc(sizeof(SDL_Cursor*) * num_cursors);

    for (int i = 0; i < num_cursors; i++) {
        cursors[i] = create_cursor(RESOURCE_GFF_INDEX, GFF_ICON, ds1_icon_res[i]);
    }

    SDL_SetCursor(cursors[0]);

    item_data = NULL;
    item_spr = SPRITE_ERROR;
    item_cursor = NULL;
}

void mouse_free() {
    for (int i = 0; i < num_cursors; i++) {
        SDL_FreeCursor(cursors[i]);
    }
    num_cursors = 0;
    free(cursors);
    cursors = NULL;
    if (item_cursor) {
        SDL_FreeCursor(item_cursor);
        item_cursor = NULL;
    }
    if (item_data) {
        free(item_data);
        item_data = NULL;
        sprite_free(item_spr);
        item_spr = SPRITE_ERROR;
    }
}

void mouse_set_as_item(ds1_item_t *item, uint16_t spr) {
    if (item_data) { return; } // We don't take an item if we already have one.

    item_data = item;
    item_spr = spr;

    // Unforutnately, in SDL you are suppose to use a surface for the cursor.
    // So, we will store the texture for later and load the mouse as a surface.
    if (item_cursor) { SDL_FreeCursor(item_cursor); }
    item_cursor = create_cursor(OBJEX_GFF_INDEX, GFF_BMP, ds_item_get_bmp_id(item));
    SDL_SetCursor(item_cursor);
}

ds1_item_t* mouse_get_item() {
    return item_data;
}

ds1_item_t* mouse_retreive_item(uint16_t *spr) {
    ds1_item_t *ret = item_data;
    if (!item_data) { return NULL; }

    *spr = item_spr;
    SDL_SetCursor(cursors[0]);
    SDL_FreeCursor(item_cursor);
    item_cursor = NULL;
    item_data = NULL;

    return ret;
}
