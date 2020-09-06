#include <stdlib.h>
#include "map.h"
#include "animate.h"
#include "../src/dsl.h"
#include "../src/trigger.h"
#include "../src/dsl-execute.h"
#include "../src/dsl-manager.h"
#include "../src/dsl-object.h"
#include "../src/dsl-region.h"
#include "../src/dsl-scmd.h"
#include "../src/dsl-var.h"

static map_t *cmap = NULL;
static SDL_Renderer *cren = NULL;

void map_init(map_t *map) {
    map->tiles = NULL;
}

void map_free(map_t *map) {
    if (map->tiles) {
        for (int i = 0; i < cmap->region->num_tiles; i++) {
            SDL_DestroyTexture(map->tiles[i]);
        }
        free(map->tiles);
        map->tiles = NULL;
    }
}

int cmap_is_block(const int row, const int column) {
    if (!cmap) { return 0; }
    return dsl_region_is_block(cmap->region, row, column);
}

int cmap_is_actor(const int row, const int column) {
    if (!cmap) { return 0; }
    return dsl_region_is_actor(cmap->region, row, column);
}

int cmap_is_danger(const int row, const int column) {
    if (!cmap) { return 0; }
    return dsl_region_is_danger(cmap->region, row, column);
}

void map_load_region(map_t *map, SDL_Renderer *renderer, int id) {
    SDL_Surface* tile = NULL;
    uint32_t *ids = NULL;
    uint32_t width, height;
    unsigned char *data;
    region_object_t *obj;

    map_free(map);
    cren = renderer;
    map->region = dsl_load_region(id);
    ids = map->region->ids;
    animate_clear();
    map->tiles = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * map->region->num_tiles);

    for (int i = 0; i < map->region->num_tiles; i++) {
        dsl_region_get_tile(map->region, i, &width, &height, &data);

        tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        map->tiles[ids[i]] = SDL_CreateTextureFromSurface(renderer, tile);
        SDL_FreeSurface(tile);

        free(data);
    }

    region_list_for_each(map->region->list, obj) {
        obj->anim = animate_add(map, renderer, obj);
    }

    cmap = map;

    dsl_change_region(42);
}

void map_render(void *data, SDL_Renderer *renderer) {
    const int stretch = 2;
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    SDL_Rect tile_loc = { -xoffset, -yoffset, stretch * 16, stretch * 16 };
    uint32_t tile_id = 0;
    //map_t *map = (map_t*) data;
    map_t *map = cmap;

    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear(renderer);

    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            //tile_id = get_tile_id(map->gff_file, x, y);
            tile_id = region_tile_id(cmap->region, x, y);
            if (tile_id >= 0) {
               SDL_RenderCopy(renderer, map->tiles[tile_id], NULL, &tile_loc);
               tile_loc.x += stretch * 16;
            }
        }
        tile_loc.x = -xoffset;
        tile_loc.y += stretch * 16;
    }
    animate_render(NULL, renderer);
}

void port_swap_objs(int obj_id, int bmp_id) {
    animate_t *anim = NULL;
    SDL_Rect loc;
    region_object_t *obj;

    region_list_for_each(cmap->region->list, obj) {
        if (obj->disk_idx == obj_id) {
            anim = animate_find(obj);
            if (anim) {
                obj->btc_idx = bmp_id;
                SDL_DestroyTexture(anim->textures[0]);
                anim->textures[0] = 
                    create_texture(cren, OBJEX_GFF_INDEX, GT_BMP, obj->btc_idx, obj->bmp_idx, cmap->region->palette_id, &loc);
            } else {
                error("Unable to find animation for obj_id!\n");
            }
            return;
        }
    }
    error("Did not find obj %d to swap with!\n", obj_id);
}

#define CLICKABLE (0x10)

region_object_t* get_object_at_location(const uint32_t x, const uint32_t y) {
    const int stretch = 2;
    uint32_t mx = getCameraX() + x;
    uint32_t my = getCameraY() + y;
    uint32_t osx, osy, oex, oey; // object start x, object start y, object end x, object end y
    SDL_Rect loc;
    region_object_t *obj = NULL;
    if (!cmap) { return 0; }

    region_list_for_each(cmap->region->list, obj) {
        if (obj->flags & CLICKABLE) {
            animate_t *anim = obj->anim;
            if (anim && anim->obj) {
                loc.w = anim->obj->bmp_width;
                loc.h = anim->obj->bmp_height;
                loc.x = anim->obj->mapx;
                loc.y = anim->obj->mapy;
                osx = loc.x * stretch;
                osy = loc.y * stretch;
                oex = osx + (stretch * loc.w);
                oey = osy + (stretch * loc.h);
            } else {
                osx = obj->mapx * stretch;
                osy = obj->mapy * stretch;
                oex = osx + (stretch * obj->bmp_width);
                oey = osy + (stretch * obj->bmp_height);
            }
            if (mx >= osx && mx < oex && my >= osy && my < oey) {
                return obj;
            }
        }
    }

    return NULL;
}

int map_handle_mouse(const uint32_t x, const uint32_t y) {
    region_object_t *obj = NULL;

    if (!cmap) { return 0; }
    obj = get_object_at_location(x, y);

    if (obj) {
        //debug("found @ %d, need to change icon\n", obj_id);
    }

    // Nothing found...

    return 1; // map always intercepts the mouse...
}

int map_handle_mouse_click(const uint32_t x, const uint32_t y) {
    region_object_t *obj = NULL;

    if (!cmap) { return 0; }
    obj = get_object_at_location(x, y);

    //print_all_checks();

    // Right now we assume all icons are talking, will look at attack later.
    if (obj) {
        gff_map_object_t* mo = get_map_object(cmap->region->gff_file, cmap->region->map_id, obj->entry_id);
        debug("Clicked on object: %d\n", abs(mo->index));
        talk_click(mo->index);
        //dsl_check_t* check = dsl_find_check(TALK_TO_CHECK_INDEX, mo->index);
        /*
        talkto_trigger_t tt = get_talkto_trigger(mo->index);
        look_trigger_t lt = get_look_trigger(mo->index);
        //printf("---------->tt.obj = %d\n", tt.obj);
        //printf("---------->lt.obj = %d\n", lt.obj);
        if (tt.obj == mo->index) {
            dsl_lua_execute_script(tt.file, tt.addr, 0);
        }
        if (lt.obj == mo->index) {
            dsl_lua_execute_script(lt.file, lt.addr, 0);
        }
        */
        //if (check) {
            //debug("TALK CHECK: Need to execute file = %d, addr = %d, global = %d\n",
                //check->data.name_check.file, check->data.name_check.addr,
                //check->data.name_check.global);
            //dsl_execute_subroutine(check->data.name_check.file,
                //check->data.name_check.addr, 0);
        //}
        //check = dsl_find_check(LOOK_CHECK_INDEX, mo->index);
        //if (check) {
            //debug("LOOK CHECK: Need to execute file = %d, addr = %d, global = %d\n",
                //check->data.name_check.file, check->data.name_check.addr,
                //check->data.name_check.global);
            //dsl_execute_subroutine(check->data.name_check.file,
                //check->data.name_check.addr, 0);
        //}
        //debug("manually calling %d: %d\n", 5, 341);
            //dsl_execute_subroutine(5, 341, 0);
        //debug("Searching other checks...\n");
        //for (int i = 1; i < MAX_CHECK_TYPES; i++) {
            //debug("check[%d] = %p\n", i, dsl_find_check(i, mo->index));
        //}
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
