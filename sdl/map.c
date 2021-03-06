#include <stdlib.h>
#include "map.h"
#include "animate.h"
#include "sprite.h"
#include "main.h"
#include "player.h"
#include "screens/narrate.h"
#include "screens/combat-status.h"
#include "../src/dsl.h"
#include "../src/port.h"
#include "../src/trigger.h"
#include "../src/dsl-manager.h"
#include "../src/ds-object.h"
#include "../src/region-manager.h"
#include "../src/ds-scmd.h"
#include "../src/ds-player.h"
#include "../src/dsl-var.h"

static map_t *cmap = NULL;
static SDL_Renderer *cren = NULL;
static animate_sprite_t anims[256];
static animate_sprite_node_t *anim_nodes[256];
static int anim_pos = 0;

static void clear_animations();
void map_free(map_t *map);

void map_init(map_t *map) {
    map->tiles = NULL;
}

void map_cleanup() {
    clear_animations();
    map_free(cmap);
    cmap = NULL;
}

void map_free(map_t *map) {
    if (!map) { return; }
    if (map->tiles) {
        for (int i = 0; i < cmap->new_region->num_tiles; i++) {
            if (map->tiles[i]) {
                SDL_DestroyTexture(map->tiles[i]);
            }
        }
        free(map->tiles);
        map->tiles = NULL;
    }
    region_free(map->new_region);
    dsl_region_free(map->region);
    free(map);
}

int cmap_is_block(const int row, const int column) {
    return region_manager_get_current()->flags[row][column];
}

int cmap_is_actor(const int row, const int column) {
    if (!cmap) { return 0; }
    return dsl_region_is_actor(cmap->region, row, column);
}

int cmap_is_danger(const int row, const int column) {
    if (!cmap) { return 0; }
    return dsl_region_is_danger(cmap->region, row, column);
}


static void map_load_current_region() {
    SDL_Surface* tile = NULL;
    uint32_t *ids = NULL;
    uint32_t width, height;
    size_t max_id = 0;
    unsigned char *data;
    //region_object_t *obj;
    gff_palette_t *pal;
    map_t *map = cmap;

    pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + map->new_region->map_id - 1;
    ids = cmap->new_region->tile_ids;
    for (int i = 0; i < map->new_region->num_tiles; i++) { max_id = max_id > ids[i] ? max_id : ids[i]; }
    max_id++;
    map->tiles = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * max_id);
    memset(map->tiles, 0x0, sizeof(SDL_Texture*) * max_id);

    for (int i = 0; i < map->new_region->num_tiles; i++) {
        region_get_tile(map->new_region, i, &width, &height, &data);

        tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

        map->tiles[ids[i]] = SDL_CreateTextureFromSurface(cren, tile);
        SDL_FreeSurface(tile);

        free(data);
    }

    dude_t *dude;
    entity_list_for_each(map->new_region->entities, dude) {
        port_add_entity(dude, pal);
    }

    cmap = map;
}

static map_t *create_map() {
    map_t *ret = malloc(sizeof(map_t));
    memset(ret, 0x0, sizeof(map_t));
    return ret;
}

void map_load_region(region_t *reg, SDL_Renderer *renderer) {
    map_free(cmap);
    if (!cmap) { cmap = create_map(); }
    map_init(cmap);
    cren = renderer;
    cmap->new_region = reg;
    //cmap->new_region = region_manager_get_region(reg->map_id);
    map_load_current_region();

    // TODO: NEED TO CLEAR ALL SCREENS
    screen_push_screen(renderer, &map_screen, 0, 0);
    screen_push_screen(renderer, &narrate_screen, 0, 0);
}

void map_load_map(SDL_Renderer *renderer, int id) {
    map_free(cmap);
    if (!cmap) { cmap = create_map(); }

    cren = renderer;
    cmap->new_region = region_manager_get_region(id);

    map_load_current_region();
    dsl_change_region(42);
}

static void clear_animations() {
    for (int i = 0; i < anim_pos; i++) {
        animate_list_remove(anim_nodes[i], anim_nodes[i]->anim->entity ? anim_nodes[i]->anim->entity->mapz : 0);
        sprite_free(anims[i].spr);
        anims[i].spr = SPRITE_ERROR;
        anim_nodes[i] = NULL;
    }

    anim_pos = 0;
}

void map_render(void *data, SDL_Renderer *renderer) {
    const int stretch = 2;
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    SDL_Rect tile_loc = { -xoffset, -yoffset, stretch * 16, stretch * 16 };
    uint32_t tile_id = 0;
    map_t *map = cmap;

    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear(renderer);

    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            //tile_id = region_tile_id(cmap->region, x, y);
            tile_id = cmap->new_region->tiles[x][y];
            if (tile_id >= 0) {
               SDL_RenderCopy(renderer, map->tiles[tile_id], NULL, &tile_loc);
               tile_loc.x += stretch * 16;
            }
        }
        tile_loc.x = -xoffset;
        tile_loc.y += stretch * 16;
    }
    animate_list_render(renderer);
}

void port_add_entity(entity_t *entity, gff_palette_t *pal) {
    const float zoom = main_get_zoom();

    anims[anim_pos].scmd = entity->sprite.scmd;
    anims[anim_pos].spr =
        sprite_new(cren, pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, entity->sprite.bmp_id);
    if (entity->name) { // If it is a combat entity, then we need to add the combat sprites
        sprite_append(anims[anim_pos].spr, cren, pal, 0, 0, zoom,
            OBJEX_GFF_INDEX, GFF_BMP, entity->sprite.bmp_id + 1);
    }
    anims[anim_pos].delay = 0;
    anims[anim_pos].pos = 0;
    anims[anim_pos].x = (entity->mapx * 16 + entity->sprite.xoffset) * zoom;
    anims[anim_pos].y = (entity->mapy * 16 + entity->sprite.yoffset + entity->mapz) * zoom;
    anims[anim_pos].destx = anims[anim_pos].x;
    anims[anim_pos].desty = anims[anim_pos].y;
    anims[anim_pos].move = anims[anim_pos].left_over = 0.0;
    anims[anim_pos].entity = entity;
    anim_nodes[anim_pos] = animate_list_add(anims + anim_pos, entity->mapz);
    entity->sprite.data = anims + anim_pos;
    anim_nodes[anim_pos]->anim->entity = entity;
    //entity->data = anims + anim_pos;
    //printf("%d: %d %d %d (%d, %d)\n", obj->combat_id, obj->mapx, obj->mapy, obj->mapz, anims[anim_pos].x, anims[anim_pos].y);
    //printf("             (%d, %d)\n", anims[anim_pos].destx, anims[anim_pos].desty);
    anim_pos++;
}

void port_add_obj(region_object_t *obj, gff_palette_t *pal) {
    const float zoom = main_get_zoom();

    anims[anim_pos].scmd = obj->scmd;
    anims[anim_pos].spr = 
        sprite_new(cren, pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, obj->btc_idx);
    anims[anim_pos].delay = 0;
    anims[anim_pos].pos = 0;
    anims[anim_pos].x = obj->bmpx * zoom;
    anims[anim_pos].y = obj->bmpy * zoom;
    anims[anim_pos].destx = anims[anim_pos].x;
    anims[anim_pos].desty = anims[anim_pos].y;
    anims[anim_pos].move = anims[anim_pos].left_over = 0.0;
    anims[anim_pos].obj = obj;
    anim_nodes[anim_pos] = animate_list_add(anims + anim_pos, obj->mapz);
    obj->data = anims + anim_pos;
    //printf("%d: %d %d %d (%d, %d)\n", obj->combat_id, obj->mapx, obj->mapy, obj->mapz, anims[anim_pos].x, anims[anim_pos].y);
    //printf("             (%d, %d)\n", anims[anim_pos].destx, anims[anim_pos].desty);
    anim_pos++;
}

void port_update_entity(entity_t *entity, const uint16_t xdiff, const uint16_t ydiff) {
    animate_sprite_t *as = (animate_sprite_t*) entity->sprite.data;
    //printf("cur:%d %d\n", as->x, as->y);
    //printf("dest: %d, %d\n", as->destx, as->desty);
    as->x = as->destx;
    as->y = as->desty;
    entity->mapx += xdiff;
    entity->mapy += ydiff;
    as->destx = entity->mapx * 16 * main_get_zoom();
    as->desty = entity->mapy * 16 * main_get_zoom();
    //as->desty -= sprite_geth(as->spr) - (8 * main_get_zoom());
    animate_set_animation(as, entity->sprite.scmd, 20);
}

void port_update_obj(region_object_t *robj, const uint16_t xdiff, const uint16_t ydiff) {
    animate_sprite_t *as = (animate_sprite_t*) robj->data;
    //printf("cur:%d %d\n", as->x, as->y);
    //printf("dest: %d, %d\n", as->destx, as->desty);
    as->x = as->destx;
    as->y = as->desty;
    robj->mapx += xdiff;
    robj->mapy += ydiff;
    as->destx = robj->mapx * 16 * main_get_zoom();
    as->desty = robj->mapy * 16 * main_get_zoom();
    //as->desty -= sprite_geth(as->spr) - (8 * main_get_zoom());
    animate_set_animation(as, robj->scmd, 20);
}

void port_enter_combat() {
    //gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    // Right now we need to migrate player to combat, we will see if that is better.
    //player_remove_animation();
    // Need to disperse players (and setup combat items.)
    // bring up combat status.
    screen_push_screen(main_get_rend(), &combat_status_screen, 295, 5);
    for (int i = 0; i < 4; i++) {
        if (i != ds_player_get_active()) {
            player_add_to_animation_list(i);
        }
    }
}

void port_exit_combat() {
    // condense players.
    // remove combat status.
    // assign experience.
}

void port_swap_enitity(int obj_id, entity_t *dude) {
    animate_sprite_t *as = (animate_sprite_t*) dude->sprite.data;
    gff_palette_t *pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + cmap->new_region->map_id - 1;
    const int zoom = 2.0;

    if (as) {
        sprite_free(as->spr);
        as->spr = sprite_new(cren, pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, dude->sprite.bmp_id);
    } else {
        error("Unable to find animation for obj_id!\n");
    }
}

void port_swap_objs(int obj_id, region_object_t *obj) {
    animate_sprite_t *as = (animate_sprite_t*) obj->data;
    gff_palette_t *pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + cmap->region->map_id - 1;
    const int zoom = 2.0;

    if (as) {
        sprite_free(as->spr);
        as->spr = sprite_new(cren, pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, obj->btc_idx);
    } else {
        error("Unable to find animation for obj_id!\n");
    }
}

#define CLICKABLE (0x10)
entity_t* get_entity_at_location(const uint32_t x, const uint32_t y) {
    entity_t *dude = NULL;
    if (!cmap) { return 0; }

    entity_list_for_each(cmap->new_region->entities, dude) {
        animate_sprite_t *as = dude->sprite.data;
        //printf("%d: %d, %d (%d, %d)\n", dude->ds_id, dude->mapx * 16, dude->mapy * 16, x, y);
        if (as && dude->object_flags & CLICKABLE) {
            if (sprite_in_rect(as->spr, x, y)) {
                return dude;
            }
        }
    }

    return NULL;
}

int map_handle_mouse(const uint32_t x, const uint32_t y) {
    //region_object_t *obj = NULL;

    if (!cmap) { return 0; }
    //obj = get_object_at_location(x, y);

    //if (obj) {
        ////debug("found @ %d, need to change icon\n", obj_id);
    //}

    return 1; // map always intercepts the mouse...
}

int map_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    dude_t *dude = NULL;

    if (!cmap) { return 0; }

    if ((dude = get_entity_at_location(x, y))) {
        talk_click(dude->ds_id);
    }
    //printf("No dude there bruh.\n");

    return 1; // map always intercepts the mouse...
}

sops_t map_screen = {
    .init = NULL,
    .cleanup = map_cleanup,
    .render = map_render,
    .mouse_movement = map_handle_mouse,
    .mouse_down = map_handle_mouse_down,
    .mouse_up = NULL,
    .data = NULL
};
