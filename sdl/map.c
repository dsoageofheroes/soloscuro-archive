#include <stdlib.h>
#include "map.h"
#include "../src/dsl.h"
#include "../src/dsl-execute.h"
#include "../src/dsl-scmd.h"
#include "../src/dsl-var.h"

map_t *cmap = NULL;

void map_init(map_t *map) {
    map->tiles = NULL;
    map->objs = NULL;
    map->num_tiles = 0;
    map->gff_file = 0;
    map->map_id = 0;
}

void map_free(map_t *map) {
    if (map->tiles) {
        for (int i = 0; i < map->num_tiles; i++) {
            SDL_DestroyTexture(map->tiles[i]);
        }
        free(map->tiles);
        map->tiles = NULL;
    }
}

void map_load_region(map_t *map, SDL_Renderer *renderer, int id) {
    SDL_Surface* tile = NULL;
    map->num_tiles = gff_get_gff_type_length(id, GT_TILE);
    uint32_t *ids = gff_get_id_list(id, GT_TILE);
    uint32_t palette_id = -1;
    int32_t width, height;
    uint16_t x, y;
    uint8_t z;
    unsigned char *data;
    uint32_t *tids = gff_get_id_list(id, GT_ETAB); // temporary to find current id for palette!

    if (!tids) { free(ids); error("Unable to find current id for map\n"); return; }
    map->map_id = *tids;
    palette_id = gff_get_palette_id(DSLDATA_GFF_INDEX, map->map_id - 1);
    free(tids);
    tids = NULL;

    map_free(map);
    gff_load_map(id);
    map->tiles = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * map->num_tiles);
    map->gff_file = id;

    for (int i = 0; i < map->num_tiles; i++) {
        width = get_frame_width(map->gff_file, GT_TILE, ids[i], 0);
        height = get_frame_height(map->gff_file, GT_TILE, ids[i], 0);
        data = get_frame_rgba_with_palette(map->gff_file, GT_TILE, ids[i], 0, palette_id);

        tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        map->tiles[ids[i]] = SDL_CreateTextureFromSurface(renderer, tile);
        SDL_FreeSurface(tile);

        free(data);
    }

    map->num_objs = gff_map_get_num_objects(map->gff_file, map->map_id);
    map->objs = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * map->num_objs);
    map->obj_locs = (SDL_Rect*) malloc(sizeof(SDL_Rect) * map->num_objs);
    map->flags = (uint16_t*) malloc(sizeof(uint16_t) * map->num_objs);
    //map->num_objs = 32;
    for (int i = 0; i < map->num_objs; i++) {
        scmd_t *scmd = gff_map_get_object_scmd(map->gff_file, map->map_id, i, 0);
        if (!scmd) {
            data = gff_map_get_object_bmp_pal(map->gff_file, map->map_id, i, &width, &height, 0, palette_id);
            tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            map->objs[i] = SDL_CreateTextureFromSurface(renderer, tile);
            map->flags[i] = gff_map_get_object_location(map->gff_file, map->map_id, i, &x, &y, &z);
            map->obj_locs[i].w = width;
            map->obj_locs[i].h = height;
            map->obj_locs[i].x = x;
            map->obj_locs[i].y = y;
        } else {
            data = gff_map_get_object_bmp_pal(map->gff_file, map->map_id, i, &width, &height, 0, palette_id);
            tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
            //gff_map_object_t* mo = get_map_object(map->gff_file, map->map_id, i);
            //disk_object_t* dobj = gff_get_object(mo->index);
            //dsl_scmd_print(OBJEX_GFF_INDEX, dobj->script_id);
            // FIXME:SCMD need to be added...
            map->objs[i] = NULL;
            debug("scmd[%d]: %d, %d\n", i, scmd->xoffset, scmd->yoffset);
            debug("scmd[%d]: %d, %d\n", i, scmd->xoffsethot, scmd->yoffsethot);
            map->objs[i] = SDL_CreateTextureFromSurface(renderer, tile);
            map->flags[i] = gff_map_get_object_location(map->gff_file, map->map_id, i, &x, &y, &z);
            debug("%d: flags = %x\n", i, map->flags[i]);
            map->obj_locs[i].w = width;
            map->obj_locs[i].h = height;
            map->obj_locs[i].x = x;
            map->obj_locs[i].y = y;
            //map->obj_locs[i].x += (scmd->xoffsethot);
            //map->obj_locs[i].y += (scmd->yoffsethot);
        }
        free(data);
    }

    cmap = map;

    dsl_change_region(42);

    free(ids);
}

void map_render(void *data, SDL_Renderer *renderer) {
    const int stretch = 2;
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    SDL_Rect tile_loc = { -xoffset, -yoffset, stretch * 16, stretch * 16 };
    SDL_Rect obj_loc = { 0, 0, 0, 0 };
    uint32_t tile_id = 0;
    map_t *map = (map_t*) data;

    //SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear(renderer);

    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            tile_id = get_tile_id(map->gff_file, x, y);
            if (tile_id >= 0) {
               SDL_RenderCopy(renderer, map->tiles[tile_id], NULL, &tile_loc);
               tile_loc.x += stretch * 16;
            }
        }
        tile_loc.x = -xoffset;
        tile_loc.y += stretch * 16;
    }
    for (int i = 0; i < map->num_objs; i++) {
        //gff_map_object_t* mo = get_map_object(map->gff_file, map->map_id, i);
        //disk_object_t* dobj = gff_get_object(mo->index);
        //printf("dobj = %p\n", dobj);
        //if (dobj && dobj->flags) {
            //printf("[%d]: dobj->flags = 0x%x\n", i, dobj->flags);
        //}
        memcpy(&obj_loc, map->obj_locs+i, sizeof(SDL_Rect));
        obj_loc.x *= 2;
        obj_loc.x -= xoffset;
        obj_loc.y *= 2;
        obj_loc.y -= yoffset;
        obj_loc.w *= stretch;
        obj_loc.h *= stretch;
        SDL_RenderCopy(renderer, map->objs[i], NULL, &obj_loc);
        //SDL_RenderCopyEx(renderer, map->objs[i], NULL, &obj_loc, 0.0, NULL, SDL_FLIP_HORIZONTAL);
    }
}

#define CLICKABLE (0x10)

int get_object_at_location(const uint32_t x, const uint32_t y) {
    const int stretch = 2;
    uint32_t mx = getCameraX() + x;
    uint32_t my = getCameraY() + y;
    uint32_t osx, osy, oex, oey; // object start x, object start y, object end x, object end y
    if (!cmap) { return 0; }

    // PERFORMANCE FIXME: should only go through needed objects, possibly quad-tree.
    for (int i = 0; i < cmap->num_objs; i++) {
        if (cmap->flags[i] & CLICKABLE) {
            osx = cmap->obj_locs[i].x * stretch;
            osy = cmap->obj_locs[i].y * stretch;
            oex = osx + (stretch * cmap->obj_locs[i].w);
            oey = osy + (stretch * cmap->obj_locs[i].h);
            //printf("cmap[%d] = (%d, %d) ->  (%d, %d -> %d, %d)\n", i, mx, my, osx, osy, oex, oey);
            if (mx >= osx && mx < oex && my >= osy && my < oey) {
                return i;
            }
        }
    }

    return -1;
}

int map_handle_mouse(const uint32_t x, const uint32_t y) {
    int obj_id = -1;

    if (!cmap) { return 0; }
    obj_id = get_object_at_location(x, y);

    if (obj_id >= 0) {
        //debug("found @ %d, need to change icon\n", obj_id);
    }

    // Nothing found...

    return 1; // map always intercepts the mouse...
}

int map_handle_mouse_click(const uint32_t x, const uint32_t y) {
    int obj_id = -1;

    if (!cmap) { return 0; }
    obj_id = get_object_at_location(x, y);

    print_all_checks();

    // Right now we assume all icons are talking, will look at attack later.
    if (obj_id >= 0) {
        gff_map_object_t* mo = get_map_object(cmap->gff_file, cmap->map_id, obj_id);
        debug("Clicked on object: %d\n", abs(mo->index));
        dsl_check_t* check = dsl_find_check(TALK_TO_CHECK_INDEX, mo->index);
        if (check) {
            debug("TALK CHECK: Need to execute file = %d, addr = %d, global = %d\n",
                check->data.name_check.file, check->data.name_check.addr,
                check->data.name_check.global);
            dsl_execute_subroutine(check->data.name_check.file,
                check->data.name_check.addr, 0);
        }
        check = dsl_find_check(LOOK_CHECK_INDEX, mo->index);
        if (check) {
            debug("LOOK CHECK: Need to execute file = %d, addr = %d, global = %d\n",
                check->data.name_check.file, check->data.name_check.addr,
                check->data.name_check.global);
            dsl_execute_subroutine(check->data.name_check.file,
                check->data.name_check.addr, 0);
        }
        //debug("manually calling %d: %d\n", 5, 341);
            //dsl_execute_subroutine(5, 341, 0);
        debug("Searching other checks...\n");
        for (int i = 1; i < MAX_CHECK_TYPES; i++) {
            debug("check[%d] = %p\n", i, dsl_find_check(i, mo->index));
        }
    }
    return 1; // map always intercepts the mouse...
}

sops_t map_screen = {
    .init = NULL,
    .render = map_render,
    .mouse_movement = map_handle_mouse,
    .mouse_click = map_handle_mouse_click,
    .data = NULL
};
