#include "mouse.h"
#include "../src/gff.h"
#include "../src/gfftypes.h"

const static size_t icon_res[] = {
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

static SDL_Cursor* create_cursor(const size_t res_id) {
    SDL_Surface *surface = NULL;
    SDL_Cursor *c;
    SDL_Rect loc;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    unsigned char *data = get_frame_rgba_palette(RESOURCE_GFF_INDEX, GFF_ICON, res_id, 0, pal);
    loc.w = get_frame_width(RESOURCE_GFF_INDEX, GFF_ICON, res_id, 0);
    loc.h = get_frame_height(RESOURCE_GFF_INDEX, GFF_ICON, res_id, 0);
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
    c = SDL_CreateColorCursor(stretch, 8, 8);
    SDL_FreeSurface(surface);
    SDL_FreeSurface(stretch);
    return c;
}

void mouse_init(SDL_Renderer *rend) {
    num_cursors = sizeof(icon_res) / sizeof(size_t);
    cursors = malloc(sizeof(SDL_Cursor*) * num_cursors);

    for (int i = 0; i < num_cursors; i++) {
        cursors[i] = create_cursor(icon_res[i]);
    }

    SDL_SetCursor(cursors[0]);
}

void mouse_free() {
    for (int i = 0; i < num_cursors; i++) {
        SDL_FreeCursor(cursors[i]);
    }
    num_cursors = 0;
    free(cursors);
    cursors = NULL;
}
