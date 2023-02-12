#include "mouse.h"
#include "gpl.h"
#include "gff.h"
#include "gfftypes.h"

#include <SDL2/SDL.h>

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
static sol_item_t *item_data = NULL;
static power_t *power = NULL;
static SDL_Cursor *item_cursor;
static SDL_Cursor *power_cursor;
static enum sol_mouse_state_e state = MOUSE_POINTER;

extern sol_status_t sol_mouse_get_state(enum sol_mouse_state_e *ms) {
    if (!ms) { return SOL_NULL_ARGUMENT; }
    *ms = state;
    return SOL_SUCCESS;
}

extern sol_status_t sol_mouse_set_state(const enum sol_mouse_state_e _state) {
    switch(_state) {
        case MOUSE_POINTER:    SDL_SetCursor(cursors[0]); break;
        case MOUSE_NO_POINTER: SDL_SetCursor(cursors[1]); break;
        case MOUSE_MELEE:      SDL_SetCursor(cursors[2]); break;
        case MOUSE_NO_MELEE:   SDL_SetCursor(cursors[3]); break;
        case MOUSE_RANGE:      SDL_SetCursor(cursors[4]); break;
        case MOUSE_NO_RANGE:   SDL_SetCursor(cursors[5]); break;
        case MOUSE_TALK:       SDL_SetCursor(cursors[6]); break;
        case MOUSE_NO_TALK:    SDL_SetCursor(cursors[7]); break;
        case MOUSE_NO_POWER:   SDL_SetCursor(cursors[8]); break;
        case MOUSE_WAIT:       SDL_SetCursor(cursors[9]); break;
        case MOUSE_ITEM:
            if (!item_cursor) {
                error ("Trying to set item cursor, but no item selected!\n");
                return SOL_NOT_FOUND;
            }
            SDL_SetCursor(item_cursor);
            break;
        case MOUSE_POWER:
            if (!power_cursor) {
                error ("Trying to set power cursor, but no power selected!\n");
                return SOL_NOT_FOUND;
            }
            SDL_SetCursor(power_cursor);
            break;
        default:
            error ("Unknown mouse state requested: %d\n", _state);
            return SOL_UNKNOWN_ERROR;
    }
    state = _state;
    return SOL_SUCCESS;
}

static SDL_Cursor* create_cursor(const int gff_idx, const int type_idx, const size_t res_id) {
    SDL_Surface *surface = NULL;
    SDL_Cursor *c;
    SDL_Rect loc;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    unsigned char *data = gff_get_frame_rgba_palette(gff_idx, type_idx, res_id, 0, pal);
    loc.w = gff_get_frame_width(gff_idx, type_idx, res_id, 0);
    loc.h = gff_get_frame_height(gff_idx, type_idx, res_id, 0);
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

extern sol_status_t sol_mouse_init() {
    if (gff_get_game_type() != DARKSUN_1) {
        warn("Mouse cursor implemented in DS1 only right now. Not loading mouse cursor.");
        return SOL_NOT_IMPLEMENTED;
    }
    num_cursors = sizeof(ds1_icon_res) / sizeof(size_t);
    cursors = malloc(sizeof(SDL_Cursor*) * num_cursors);

    for (int i = 0; i < num_cursors; i++) {
        cursors[i] = create_cursor(RESOURCE_GFF_INDEX, GFF_ICON, ds1_icon_res[i]);
    }

    SDL_SetCursor(cursors[0]);

    item_data = NULL;
    item_cursor = NULL;
    power_cursor = NULL;
    return SOL_SUCCESS;
}

extern sol_status_t sol_mouse_free() {
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
        sol_item_free(item_data);
        item_data = NULL;
    }
    if (power_cursor) {
        SDL_FreeCursor(power_cursor);
        power_cursor = NULL;
    }
    return SOL_SUCCESS;
}

extern sol_status_t sol_mouse_set_as_item(sol_item_t *item) {
    if (!item) { return SOL_NULL_ARGUMENT; }
    if (item_data) { 
        sol_item_free_except_graphics(item_data);
        item_data = NULL;
    }

    sol_item_dup(item, &item_data);

    // Unfortunately, in SDL you are suppose to use a surface for the cursor.
    // So, we will store the texture for later and load the mouse as a surface.
    if (item_cursor) { SDL_FreeCursor(item_cursor); }
    item_cursor = create_cursor(OBJEX_GFF_INDEX, GFF_BMP, item->anim.bmp_id);
    sol_mouse_set_state(MOUSE_ITEM);
    return SOL_SUCCESS;
}

extern sol_status_t sol_mouse_get_item(sol_item_t **item) {
    if (!item) { return SOL_NULL_ARGUMENT; }

    *item = item_data;
    return SOL_SUCCESS;
}

extern sol_status_t sol_mouse_free_item() {
    if (item_data) {
        free(item_data);
        item_data = NULL;
    }

    if (item_cursor) { SDL_FreeCursor(item_cursor); }

    SDL_SetCursor(cursors[0]);
    return SOL_SUCCESS;
}

extern sol_status_t sol_mouse_get_power(power_t **p) {
    if (!p) { return SOL_NULL_ARGUMENT; }
    *p = power;
    return SOL_SUCCESS;
}

extern void sol_mouse_set_as_power(power_t *pw) {
    if (!pw) { return; }
    if (power_cursor) { SDL_FreeCursor(power_cursor); }

    power = pw;
    power_cursor = create_cursor(RESOURCE_GFF_INDEX, GFF_ICON, pw->icon_id);
    sol_mouse_set_state(MOUSE_POWER);
}
