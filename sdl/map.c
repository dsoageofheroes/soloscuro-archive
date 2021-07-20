#include <stdlib.h>
#include "map.h"
#include "animate.h"
#include "sprite.h"
#include "main.h"
#include "player.h"
#include "audio.h"
#include "mouse.h"
#include "windows/narrate.h"
#include "windows/combat-status.h"
#include "../src/dsl.h"
#include "../src/port.h"
#include "../src/trigger.h"
#include "../src/dsl-manager.h"
#include "../src/ssi-object.h"
#include "../src/region-manager.h"
#include "../src/ssi-scmd.h"
#include "../src/player.h"
#include "../src/dsl-var.h"

#define MAX_ANIMS (256)

static map_t *cmap = NULL;
static SDL_Renderer *cren = NULL;
static animate_sprite_t anims[MAX_ANIMS];
static animate_sprite_node_t *anim_nodes[MAX_ANIMS];
static int anim_pos = 0;
static int mousex = 0, mousey = 0;
static uint16_t tile_highlight = SPRITE_ERROR;

static void clear_animations();
void map_free(map_t *map);
static void map_load_current_region();
static map_t *create_map();

static void sprite_load_animation(entity_t *entity, gff_palette_t *pal);
void map_render_anims(SDL_Renderer *renderer);

void map_load(SDL_Renderer *renderer, const uint32_t _x, const uint32_t _y) {
    if (!cmap && region_manager_get_current()) {
        cmap = create_map();
        cren = renderer;
        cmap->region = region_manager_get_current();
        map_load_current_region();
    }
}

void map_init(map_t *map) {
    map->tiles = NULL;
}

void map_cleanup() {
    clear_animations();
    map_free(cmap);
    if (tile_highlight != SPRITE_ERROR) {
        sprite_free(tile_highlight);
        tile_highlight = SPRITE_ERROR;
    }
    cmap = NULL;
}

void map_free(map_t *map) {
    if (!map) { return; }
    //TODO: unload region!
    for (uint32_t i = 0; i < map->region->num_tiles; i++) {
        if (map->tiles[i]) {
            SDL_DestroyTexture(map->tiles[i]);
            map->tiles[i] = NULL;
        }
    }
    free(map->tiles);
    map->tiles = NULL;
    free(map);
}

int cmap_is_block(const int row, const int column) {
    return region_manager_get_current()->flags[row][column];
}

static void map_load_current_region() {
    SDL_Surface* tile = NULL;
    uint32_t *ids = NULL;
    uint32_t width, height;
    size_t max_id = 0;
    unsigned char *data;
    gff_palette_t *pal = NULL;
    map_t *map = cmap;
    int offset = 0;

    if (map->region->map_id < 100 && map->region->map_id > 0) {
        pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + map->region->map_id - 1;
        offset = 1;
    }
    ids = cmap->region->tile_ids;
    max_id = 256;
    map->tiles = (SDL_Texture**) malloc(sizeof(SDL_Texture*) * max_id);
    memset(map->tiles, 0x0, sizeof(SDL_Texture*) * max_id);

    for (uint32_t i = 0; i < map->region->num_tiles; i++) {
        region_get_tile(map->region, ids[i], &width, &height, &data);

        if (data && *data) {
            tile = SDL_CreateRGBSurfaceFrom(data, width, height, 32, 4*width, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

            map->tiles[i + offset] = SDL_CreateTextureFromSurface(cren, tile);
            //map->tiles[i] = SDL_CreateTextureFromSurface(cren, tile);
            SDL_FreeSurface(tile);

            free(data);
        }
    }

    dude_t *dude;
    entity_list_for_each(map->region->entities, dude) {
        //port_add_entity(dude, pal);
        sprite_load_animation(dude, pal);
    }

    if (player_get_active()) {
        if (player_get_active()->anim.spr == SPRITE_ERROR) {
            sprite_load_animation(player_get_active(), pal);
        }
        //animation_list_add(map->region->anims, &(player_get_active()->anim));
        port_place_entity(player_get_active());
    }

    cmap = map;

    //TODO: Find out what maps to which areas.
    audio_play_xmi(RESOURCE_GFF_INDEX, GFF_GSEQ, 2);

    dsl_lua_execute_script(cmap->region->map_id, 0, 1);
}

static void sprite_load_animation(entity_t *entity, gff_palette_t *pal) {
    const float zoom = main_get_zoom();
    //anims[anim_pos].scmd = entity->sprite.anim.scmd;
    //anims[anim_pos].spr =
    entity->anim.spr =
        sprite_new(cren, pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, entity->sprite.bmp_id);
    if (entity->name) { // If it is a combat entity, then we need to add the combat sprites
        sprite_append(entity->anim.spr, cren, pal, 0, 0, zoom,
            OBJEX_GFF_INDEX, GFF_BMP, entity->sprite.bmp_id + 1);
    }
    entity->anim.delay = 0;
    entity->anim.pos = 0;
    entity->anim.w = sprite_getw(entity->anim.spr);
    entity->anim.h = sprite_geth(entity->anim.spr);
    entity->anim.x = (entity->mapx * 16 + entity->sprite.xoffset) * zoom;
    entity->anim.y = (entity->mapy * 16 + entity->sprite.yoffset + entity->mapz) * zoom;
    entity->anim.destx = entity->anim.x;
    entity->anim.destx -= sprite_getw(entity->anim.spr) / 2;
    entity->anim.desty = entity->anim.y;

    if (entity->name) {
        entity->anim.desty -= sprite_geth(entity->anim.spr) - (8 * main_get_zoom());
    }
    entity->anim.movey = entity->anim.movex = entity->anim.left_over = 0.0;
    entity->anim.entity = entity;
    //printf("%d: %d %d %d (%d, %d)\n", obj->combat_id, obj->mapx, obj->mapy, obj->mapz, anims[anim_pos].x, anims[anim_pos].y);
    //printf("             (%d, %d)\n", anims[anim_pos].destx, anims[anim_pos].desty);
    //printf("%s: @ %p\n", entity->name, entity->sprite.data);
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
    cmap->region = reg;
    //cmap->region = region_manager_get_region(reg->map_id);
    map_load_current_region();

    // TODO: NEED TO CLEAR ALL SCREENS
    window_push(renderer, &map_window, 0, 0);
    window_push(renderer, &narrate_window, 0, 0);
    window_push(renderer, &combat_status_window, 295, 5);
}

void map_load_map(SDL_Renderer *renderer, int id) {
    map_free(cmap);
    if (!cmap) { cmap = create_map(); }
    map_init(cmap);

    cren = renderer;
    cmap->region = region_manager_get_region(id);

    map_load_current_region();
    dsl_change_region(42);
}

static void clear_animations() {
    for (int i = 0; i < anim_pos; i++) {
        if (anim_nodes[i]) {
            animate_list_remove(anim_nodes[i], anim_nodes[i]->anim->entity ? anim_nodes[i]->anim->entity->mapz : 0);
            sprite_free(anims[i].spr);
            anims[i].spr = SPRITE_ERROR;
            anim_nodes[i] = NULL;
        }
    }

    anim_pos = 0;
}

void map_apply_alpha(const uint8_t alpha) {
    entity_t *entity = NULL;

    if (!cmap || !cmap->region || !cmap->tiles) { return; }

    for (uint32_t i = 0; i < cmap->region->num_tiles; i++) {
        SDL_SetTextureAlphaMod(cmap->tiles[i + 1], alpha);
    }

    entity_list_for_each(cmap->region->entities, entity) {
        if (entity->sprite.data) {
            animate_sprite_node_t *asn = (animate_sprite_node_t*) entity->sprite.data;
            if (asn) {
                sprite_set_alpha(asn->anim->spr, alpha);
            }
        }
    }
}

extern void map_highlight_tile(const int tilex, const int tiley, const int frame) {
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    const float zoom = main_get_zoom();
    const int x = tilex * (16 * zoom) - xoffset;
    const int y = tiley * (16 * zoom) - yoffset;

    sprite_set_frame(tile_highlight, frame);
    sprite_set_location(tile_highlight, x - zoom, y - zoom);
    sprite_render(main_get_rend(), tile_highlight);
}

static void show_debug_info() {
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    const float zoom = main_get_zoom();
    int x = (xoffset + mousex) / (16 * zoom);
    int y = (yoffset + mousey) / (16 * zoom);

    if (tile_highlight == SPRITE_ERROR) {
        const float zoom = main_get_zoom();
        gff_palette_t* pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
        tile_highlight = sprite_new(main_get_rend(), pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20088);
    }

    map_highlight_tile(x, y, 4);
    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            if (region_is_block(region_manager_get_current(), x, y)) {
                map_highlight_tile(y, x, 6);
            }
        }
    }
}

void map_render(void *data, SDL_Renderer *renderer) {
    const int stretch = main_get_zoom();
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    SDL_Rect tile_loc = { -xoffset, -yoffset, stretch * 16, stretch * 16 };
    uint32_t tile_id = 0;
    map_t *map = cmap;

    SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
    SDL_RenderClear(renderer);

    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            tile_id = cmap->region->tiles[x][y];
            if (tile_id >= 0) {
               SDL_RenderCopy(renderer, map->tiles[tile_id], NULL, &tile_loc);
               tile_loc.x += stretch * 16;
            }
        }
        tile_loc.x = -xoffset;
        tile_loc.y += stretch * 16;
    }

    //anim = map->region->anims->head->anim;
    //for (animation_node_t *__el_rover = (map->region->anims->head); __el_rover;
        //__el_rover = __el_rover->next, anim = __el_rover->anim) {
    //}
    map_render_anims(renderer);
    /*
    animation_list_for_each(map->region->anims, anim) {
        if (anim->spr != SPRITE_ERROR){
            //printf("render: %d\n", anim->spr);
            sprite_set_location(anim->spr,
                anim->x - xoffset, // + scmd_xoffset,
                anim->y - yoffset); // + anim->scmd->yoffset);
            sprite_render(renderer, anim->spr);
        }
    }
    */
    //animate_list_render(renderer);

    if (main_get_debug()) { show_debug_info(); }
}

//static int do_shift = 0;
/*
static SDL_RendererFlip map_animate_tick(animate_sprite_t *anim, const uint32_t xoffset, const uint32_t yoffset) {
    size_t pos = anim->pos;
    SDL_RendererFlip flip = 0;

    sprite_set_frame(anim->spr, anim->scmd[pos].bmp_idx);
    if (anim->scmd[pos].flags & SCMD_LAST) { goto out; }

    if (anim->scmd[pos].flags & SCMD_JUMP && anim->delay == 0) {
        pos = anim->pos = 0;
        sprite_set_frame(anim->spr, anim->scmd[pos].bmp_idx);
        anim->delay = anim->scmd[pos].delay;
    }

    if (anim->delay == 0) {
        anim->pos++; pos++;
        sprite_set_frame(anim->spr, anim->scmd[pos].bmp_idx);
        anim->delay = anim->scmd[pos].delay;
        goto out;
    }

    while (anim->delay == 0 && pos > 0) {
        anim->pos++; pos++;
        if (pos >= SCMD_MAX_SIZE) {
            anim->pos = pos = 0;
        }
    }

    anim->delay--;

out:
    if (anim->scmd[pos].flags & SCMD_XMIRROR
        || (anim->entity && anim->entity->sprite.flags & 0x80)) {
        flip |= SDL_FLIP_HORIZONTAL;
    }
    if (anim->scmd[pos].flags & SCMD_YMIRROR) {
        flip |= SDL_FLIP_VERTICAL;
    }

    anim->left_over += fmod(anim->movex, 1.0);
    float movex_amt = floor(anim->movex + anim->left_over);
    float movey_amt = floor(anim->movey + anim->left_over);
    anim->left_over = fmod(anim->left_over, 1.0);

    if (anim->x < anim->destx) { anim->x += movex_amt; }
    if (anim->y < anim->desty) { anim->y += movey_amt; }
    if (anim->x > anim->destx) { anim->x -= movex_amt; }
    if (anim->y > anim->desty) { anim->y -= movey_amt; }

    do_shift = !(movex_amt && movey_amt);

    //if (anim->scmd == combat_get_scmd(COMBAT_POWER_THROW_STATIC_U)) {
        //printf("%s: (%d, %d) -> (%d, %d) (move_amt = (%f, %f))\n",
            //anim->entity ? anim->entity->name : "THROW",
            //anim->x, anim->y, anim->destx, anim->desty, movex_amt, movey_amt);
    //}
    sprite_set_location(anim->spr,
        anim->x - xoffset, // + scmd_xoffset,
        anim->y - yoffset); // + anim->scmd->yoffset);

    if (main_get_debug() && anim->entity) {
        map_highlight_tile(anim->entity->mapx, anim->entity->mapy, 4);
    }

    return flip;
}
*/

void map_render_anims(SDL_Renderer *renderer) {
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    entity_t *dude;
    animate_sprite_t *anim;
    SDL_RendererFlip flip = 0;

/*
    animation_list_for_each(cmap->region->anims, anim) {
        flip = 0;
        if (anim->spr != SPRITE_ERROR){
            //printf("render: %d\n", anim->spr);
            //sprite_render(renderer, anim->spr);
            //sprite_render_flip(renderer, anim->spr, map_animate_tick(anim, xoffset, yoffset));
            if (anim->scmd[anim->pos].flags & SCMD_XMIRROR
                || (anim->entity && anim->entity->sprite.flags & 0x80)) {
                flip |= SDL_FLIP_HORIZONTAL;
            }
            if (anim->scmd[anim->pos].flags & SCMD_YMIRROR) {
                flip |= SDL_FLIP_VERTICAL;
            }
            sprite_set_location(anim->spr,
                anim->x - xoffset, // + scmd_xoffset,
                anim->y - yoffset); // + anim->scmd->yoffset);
            sprite_render_flip(renderer, anim->spr, flip);
            //if (do_shift) { animation_shift_node(__el_rover); }
        }
    }
    */
    entity_list_for_each(cmap->region->entities, dude) {
        if (dude->anim.spr == SPRITE_ERROR) { continue; }
        flip = 0;
        anim = &(dude->anim);
        if (anim->scmd[anim->pos].flags & SCMD_XMIRROR
            || (anim->entity && anim->entity->sprite.flags & 0x80)) {
            flip |= SDL_FLIP_HORIZONTAL;
        }
        if (anim->scmd[anim->pos].flags & SCMD_YMIRROR) {
            flip |= SDL_FLIP_VERTICAL;
        }
        sprite_set_location(anim->spr,
            anim->x - xoffset, // + scmd_xoffset,
            anim->y - yoffset); // + anim->scmd->yoffset);
        sprite_render_flip(renderer, anim->spr, flip);
    }
}

void port_add_entity(entity_t *entity, gff_palette_t *pal) {
    const float zoom = main_get_zoom();

    return;
    //anims[anim_pos].scmd = entity->sprite.anim.scmd;
    anims[anim_pos].spr =
        sprite_new(cren, pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, entity->sprite.bmp_id);
    if (entity->name) { // If it is a combat entity, then we need to add the combat sprites
        sprite_append(anims[anim_pos].spr, cren, pal, 0, 0, zoom,
            OBJEX_GFF_INDEX, GFF_BMP, entity->sprite.bmp_id + 1);
    }
    anims[anim_pos].delay = 0;
    anims[anim_pos].pos = 0;
    anims[anim_pos].w = sprite_getw(anims[anim_pos].spr);
    anims[anim_pos].h = sprite_geth(anims[anim_pos].spr);
    anims[anim_pos].x = (entity->mapx * 16 + entity->sprite.xoffset) * zoom;
    anims[anim_pos].y = (entity->mapy * 16 + entity->sprite.yoffset + entity->mapz) * zoom;
    anims[anim_pos].destx = anims[anim_pos].x;
    anims[anim_pos].destx -= sprite_getw(anims[anim_pos].spr) / 2;
    //anims[anim_pos].destx -= (8 * main_get_zoom());
    anims[anim_pos].desty = anims[anim_pos].y;
    if (entity->name) {
        anims[anim_pos].desty -= sprite_geth(anims[anim_pos].spr) - (8 * main_get_zoom());
    }
    anims[anim_pos].movey = anims[anim_pos].movex = anims[anim_pos].left_over = 0.0;
    anims[anim_pos].entity = entity;
    //anim_nodes[anim_pos] = animate_list_add(anims + anim_pos, entity->mapz);
    entity->sprite.data = anim_nodes[anim_pos];
    //anim_nodes[anim_pos]->anim->entity = entity;
    //printf("%d: %d %d %d (%d, %d)\n", obj->combat_id, obj->mapx, obj->mapy, obj->mapz, anims[anim_pos].x, anims[anim_pos].y);
    //printf("             (%d, %d)\n", anims[anim_pos].destx, anims[anim_pos].desty);
    //printf("%s: @ %p\n", entity->name, entity->sprite.data);
    anim_pos++;
}

void port_remove_entity(entity_t *entity) {
    if (!entity || entity->sprite.data == NULL) { return; }
    region_remove_entity(region_manager_get_current(), entity);
    for (int i = 0; i < MAX_ANIMS; i++) {
        if (anim_nodes[i] && anim_nodes[i]->anim && anim_nodes[i]->anim->entity == entity) {
            animate_list_remove(anim_nodes[i], anim_nodes[i]->anim->entity ? anim_nodes[i]->anim->entity->mapz : 0);
            sprite_free(anims[i].spr);
            //if (entity->sprite.data
            anims[i].spr = SPRITE_ERROR;
            free(anim_nodes[i]);
            anim_nodes[i] = NULL;
        }
    }
}

static void entity_instant_move(entity_t *entity) {
    animate_sprite_node_t *asn = (animate_sprite_node_t*) entity->sprite.data;
    asn->anim->x = asn->anim->destx;
    asn->anim->y = asn->anim->desty;
}

void port_place_entity(entity_t *entity) {
    animate_sprite_t *as = &(entity->anim);
    const float zoom = main_get_zoom();

    as->x = as->destx = entity->mapx * 16 * zoom;
    as->y = as->desty = entity->mapy * 16 * zoom;
    as->w = sprite_getw(entity->anim.spr);
    as->h = sprite_geth(entity->anim.spr);
    if (as->w > 16 * zoom) {
        //printf("width = %d\n", as->w);
        as->x = as->destx -= (as->w - 16 * zoom) / 2;
    }
    as->y = as->desty -= as->h - (16 * zoom);
}

void port_update_entity(entity_t *entity, const uint16_t xdiff, const uint16_t ydiff) {
    animate_sprite_t *as = &(entity->anim);
    const float zoom = main_get_zoom();
    //printf("cur:%d %d\n", as->x, as->y);
    //printf("dest: %d, %d\n", as->destx, as->desty);
    as->x = as->destx;
    as->y = as->desty;
    entity->mapx += xdiff;
    entity->mapy += ydiff;
    as->destx = entity->mapx * 16 * main_get_zoom();
    as->desty = entity->mapy * 16 * main_get_zoom();
    if (as->w > 16 * zoom) {
        //printf("width = %d\n", as->w);
        as->destx -= (as->w - 16 * zoom) / 2;
    }
    as->desty -= as->h - (16 * zoom);
}

extern void port_load_sprite(animate_sprite_t *anim, gff_palette_t *pal, const int gff_index,
                const int type, const uint32_t id, const int num_load) {
    if (!anim) { return; }

    if (anim->scmd) {
        error("port_load_sprite not implemented for sprites with a scmd!\n");
    }
    // Check if spr is already exisitng, if yes, deallocate then recreate!

    //if (anim->spr == SPRITE_ERROR) {
        //if (spr->data) {
            //free(spr->data);
            //animation_node_free(spr->data);
        //}
        //asn = animate_sprite_node_create();
        //spr->data = asn;
        //spr->anim = *(asn->anim);
        //spr->anim->spr = SPRITE_ERROR;
    //}

    if (anim->spr == SPRITE_ERROR) {
        anim->spr = sprite_new(main_get_rend(), pal, 0, 0, main_get_zoom(), gff_index, type, id);

        // Now append anything else needed.
        for (int i = 1; i < num_load; i++) {
            sprite_append(anim->spr, cren, pal, 0, 0, main_get_zoom(),
                gff_index, type, id + i);
        }
    }
    //printf("valid = %d\n", sprite_valid(asn->anim->spr));
}

extern void port_free_sprite(sprite_info_t *spr) {
    if (!spr) { return; }

    if (spr->data) {
        //animate_sprite_node_free(spr->data);
        spr->data = NULL;
    }
}

void port_enter_combat() {
    // Right now we need to migrate player to combat, we will see if that is better.
    //player_remove_animation();
    // Need to disperse players (and setup combat items.)
    // bring up combat status.
    //window_push_window(main_get_rend(), &combat_status_window, 295, 5);
    dude_t *main_player = player_get_active();
    for (int i = 0; i < 4; i++) {
        dude_t *next_player = player_get(i);
        if (next_player && next_player != player_get_active() && next_player->name) { // next_player exists.
            next_player->mapx = main_player->mapx;
            next_player->mapy = main_player->mapy;
            region_move_to_nearest(region_manager_get_current(), next_player);
            port_update_entity(next_player, 0, 0);
            entity_instant_move(next_player);
        }
    }
}

void port_exit_combat() {
    // condense players.
    player_condense();
    // remove combat status.
    //window_pop();
    // assign experience.
}

void port_swap_enitity(int obj_id, entity_t *dude) {
    animate_sprite_node_t *asn = (animate_sprite_node_t*) dude->sprite.data;
    gff_palette_t *pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + cmap->region->map_id - 1;
    const int zoom = 2.0;

    if (asn) {
        sprite_free(asn->anim->spr);
        asn->anim->spr = sprite_new(cren, pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, dude->sprite.bmp_id);
    } else {
        error("Unable to find animation for obj_id!\n");
    }
}

#define CLICKABLE (0x10)
entity_t* get_entity_at_location(const uint32_t x, const uint32_t y) {
    entity_t *dude = NULL;
    if (!cmap) { return 0; }

    entity_list_for_each(cmap->region->entities, dude) {
        animate_sprite_node_t *asn = dude->sprite.data;
        //printf("%d: %d, %d (%d, %d)\n", dude->ds_id, dude->mapx * 16, dude->mapy * 16, x, y);
        if (asn && dude->object_flags & CLICKABLE) {
            if (sprite_in_rect(asn->anim->spr, x, y)) {
                return dude;
            }
        }
    }

    return NULL;
}

static void update_mouse_icon() {
    enum mouse_state ms = mouse_get_state();
    entity_t *dude = get_entity_at_location(mousex, mousey);
    const float zoom = main_get_zoom();

    if (ms == MOUSE_MELEE || ms == MOUSE_NO_MELEE) {
        int x = (getCameraX() + mousex) / (16 * zoom);
        int y = (getCameraY() + mousey) / (16 * zoom);
        if (abs(player_get_active()->mapx - x) > 1 || abs(player_get_active()->mapy - y) > 1) {
            mouse_set_state(MOUSE_RANGE);
            ms = mouse_get_state();
        }
    } else if (ms == MOUSE_RANGE || ms == MOUSE_NO_RANGE) {
        int x = (getCameraX() + mousex) / (16 * zoom);
        int y = (getCameraY() + mousey) / (16 * zoom);
        if (abs(player_get_active()->mapx - x) <= 1 && abs(player_get_active()->mapy - y) <= 1) {
            mouse_set_state(MOUSE_MELEE);
            ms = mouse_get_state();
        }
    }

    if (!dude) {
        if (ms == MOUSE_MELEE) { mouse_set_state(MOUSE_NO_MELEE);
        } else if (ms == MOUSE_RANGE) { mouse_set_state(MOUSE_NO_RANGE);
        } else if (ms == MOUSE_TALK) { mouse_set_state(MOUSE_NO_TALK);
        } else if (ms == MOUSE_POWER) {
            switch(power_get_target_type(mouse_get_power())) {
                case TARGET_ALLY:
                case TARGET_ENEMY:
                case TARGET_MULTI:
                    mouse_set_state(MOUSE_NO_POWER);
                    break;
                default: break;
            }
        }
    } else {
        if (ms == MOUSE_NO_MELEE) { mouse_set_state(MOUSE_MELEE);
        } else if (ms == MOUSE_NO_RANGE) { mouse_set_state(MOUSE_RANGE);
        } else if (ms == MOUSE_NO_TALK) { mouse_set_state(MOUSE_TALK);
        } else if (ms == MOUSE_NO_POWER) { mouse_set_state(MOUSE_POWER);
        }
    }
}

int map_handle_mouse(const uint32_t x, const uint32_t y) {
    if (!cmap) { return 0; }

    mousex = x;
    mousey = y;

    update_mouse_icon();

    return 1; // map always intercepts the mouse...
}

// User right clicks for the next mouse pointer type;
static void mouse_cycle() {
    enum mouse_state ms = mouse_get_state();

    if (ms == MOUSE_POINTER || ms == MOUSE_NO_POINTER) {
        mouse_set_state(MOUSE_MELEE);
    } else if (ms == MOUSE_MELEE || ms == MOUSE_NO_MELEE || ms == MOUSE_RANGE || ms == MOUSE_NO_RANGE) {
        mouse_set_state(MOUSE_TALK);
    } else if (ms == MOUSE_NO_TALK || ms == MOUSE_TALK) {
        mouse_set_state((mouse_get_item()) ? MOUSE_ITEM : MOUSE_POINTER);
    } else if (ms == MOUSE_ITEM || ms == MOUSE_POWER || ms == MOUSE_NO_POWER) {
        mouse_set_state(MOUSE_POINTER);
    } else {
        error ("unable to cycle from mouse state: %d\n", ms);
    }

    update_mouse_icon();
}

int map_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    dude_t *dude = NULL;
    enum mouse_state ms = mouse_get_state();
    const uint32_t xoffset = getCameraX();
    const uint32_t yoffset = getCameraY();
    const float zoom = main_get_zoom();
    int tilex = (xoffset + mousex) / (16 * zoom);
    int tiley = (yoffset + mousey) / (16 * zoom);

    if (!cmap) { return 0; }

    if (ms == MOUSE_TALK && (dude = get_entity_at_location(x, y))) {
        mouse_set_state(MOUSE_POINTER);
        talk_click(dude->ds_id);
        return 1;
    }

    if (ms == MOUSE_POWER) {
        combat_activate_power(mouse_get_power(), player_get_active(),
            get_entity_at_location(x, y), tilex, tiley);
        mouse_set_state(MOUSE_POINTER);
        return 1;
    }

    if (button == SDL_BUTTON_RIGHT) {
        mouse_cycle();
    }
    //printf("No dude there bruh.\n");

    return 1; // map always intercepts the mouse...
}

extern void port_load_item(item_t *item) {
    //warn("Need to load item %d.\n", item->ds_id);
    if (!item) { return; }
    //animate_sprite_t *as = calloc(1, sizeof(animate_sprite_t));
    //item->sprite.data = (void*)as;
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    //as->spr = sprite_new(main_get_rend(), pal, 0, 0, main_get_zoom(),
            //OBJEX_GFF_INDEX, GFF_BMP, item->sprite.bmp_id);
    //as->entity = NULL;
    item->anim.spr = sprite_new(main_get_rend(), pal, 0, 0, main_get_zoom(),
            OBJEX_GFF_INDEX, GFF_BMP, item->sprite.bmp_id);
    item->anim.entity = NULL;
}

extern void port_free_item(item_t *item) {
    if (!item) { return; }

    if (item->anim.spr != SPRITE_ERROR) {
        sprite_free(item->anim.spr);
        item->anim.spr = SPRITE_ERROR;
    }
}

wops_t map_window = {
    .init = map_load,
    .cleanup = map_cleanup,
    .render = map_render,
    .mouse_movement = map_handle_mouse,
    .mouse_down = map_handle_mouse_down,
    .mouse_up = NULL,
    .data = NULL
};
