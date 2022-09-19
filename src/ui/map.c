#include <stdlib.h>
#include "map.h"
#include "audio.h"
#include "narrate.h"
#include "combat-status.h"
#include "background.h"
#include "gpl.h"
#include "innate.h"
#include "port.h"
#include "trigger.h"
#include "settings.h"
#include "gpl-manager.h"
#include "ssi-object.h"
#include "region-manager.h"
#include "ssi-scmd.h"
#include "player.h"
#include "examine.h"
#include "gpl-var.h"
#include "window-main.h"

#include <string.h>

#define MAX_ANIMS (256)

static map_t *cmap = NULL;
static int mousex = 0, mousey = 0;
static uint16_t tile_highlight = SPRITE_ERROR, game_over = SPRITE_ERROR;

void map_free(map_t *map);
static void map_load_current_region();
static map_t *create_map();
static dude_t *cdude = NULL;

static void sprite_load_animation(entity_t *entity, gff_palette_t *pal);
void map_render_anims();

void map_load(const uint32_t _x, const uint32_t _y) {
    if (!cmap && sol_region_manager_get_current()) {
        cmap = create_map();
        cmap->region = sol_region_manager_get_current();
        map_load_current_region();
        sol_center_on_player();
    }
}

void map_cleanup() {
    map_free(cmap);
    if (tile_highlight != SPRITE_ERROR) {
        sol_sprite_free(tile_highlight);
        tile_highlight = SPRITE_ERROR;
    }
    cmap = NULL;
}

void map_free(map_t *map) {
    if (!map) { return; }
    //TODO: unload region!
    sol_background_free();
    free(map);
}

int cmap_is_block(const int row, const int column) {
    return sol_region_manager_get_current()->flags[row][column];
}

static void map_load_current_region() {
    gff_palette_t *pal = NULL;
    map_t *map = cmap;

    sol_background_load_region(map->region);
    if (map->region->map_id < 100 && map->region->map_id > 0) {
        pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + map->region->map_id - 1;
    } else {
        pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;
    }

    dude_t *dude;
    entity_list_for_each(map->region->entities, dude) {
        if (dude->anim.spr == SPRITE_ERROR) {
            sprite_load_animation(dude, pal);
        }
    }

    if (sol_player_get_active()) {
        if (sol_player_get_active()->anim.spr == SPRITE_ERROR) {
            sprite_load_animation(sol_player_get_active(), pal);
        }
        sol_map_place_entity(sol_player_get_active());
    }

    cmap = map;

    //TODO: Find out what maps to which areas.
    sol_audio_play_xmi(RESOURCE_GFF_INDEX, GFF_GSEQ, 2);
}

static void sprite_load_animation(entity_t *entity, gff_palette_t *pal) {
    const float zoom = settings_zoom();
    entity->anim.spr =
        sol_sprite_new(pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, entity->anim.bmp_id);
    if (entity->name) { // If it is a combat entity, then we need to add the combat sprites
        sol_sprite_append(entity->anim.spr, pal, 0, 0, zoom,
            OBJEX_GFF_INDEX, GFF_BMP, entity->anim.bmp_id + 1);
    }
    entity->anim.delay = 0;
    entity->anim.w = sol_sprite_getw(entity->anim.spr);
    entity->anim.h = sol_sprite_geth(entity->anim.spr);
    entity->anim.x = (entity->mapx * 16 + entity->anim.xoffset) * zoom;
    entity->anim.y = (entity->mapy * 16 + entity->anim.yoffset + entity->mapz) * zoom;
    entity->anim.destx = entity->anim.x;
    //entity->anim.destx -= sol_sprite_getw(entity->anim.spr) / 2;
    entity->anim.desty = entity->anim.y;
    sol_sprite_set_frame(entity->anim.spr, entity->anim.load_frame);

    if (entity->name) {
        //entity->anim.desty -= sol_sprite_geth(entity->anim.spr) - (8 * settings_zoom());
    }
    entity->anim.movey = entity->anim.movex = entity->anim.left_over = 0.0;
    //printf("%d: %d %d %d (%d, %d)\n", obj->combat_id, obj->mapx, obj->mapy, obj->mapz, anims[anim_pos].x, anims[anim_pos].y);
    //printf("             (%d, %d)\n", anims[anim_pos].destx, anims[anim_pos].desty);
    //printf("%s: @ %p\n", entity->name, entity->sprite.data);
    if (sol_innate_is_door(entity) && entity->map_flags & GM_ANIMATING) {
        printf("IS DOOR and should activate: %d, map_flags = 0x%x\n", entity->ds_id, entity->map_flags);
        entity->map_flags &= ~GM_ANIMATING;
        sol_sprite_set_frame(entity->anim.spr, 2);
        sol_innate_activate_door(entity);
    }
}

static map_t *create_map() {
    map_t *ret = malloc(sizeof(map_t));
    memset(ret, 0x0, sizeof(map_t));
    return ret;
}

void map_load_region(sol_region_t *reg) {
    map_free(cmap);
    if (!cmap) { cmap = create_map(); }
    //cmap->region = region_manager_get_region(reg->map_id);
    map_load_current_region();

    // TODO: NEED TO CLEAR ALL SCREENS
    sol_window_push(&map_window, 0, 0);
    sol_window_push(&narrate_window, 0, 0);
    sol_window_push(&combat_status_window, 295, 5);
}

void map_apply_alpha(const uint8_t alpha) {
    entity_t *entity = NULL;
    if (!cmap) { return; }

    sol_background_apply_alpha(alpha);

    entity_list_for_each(cmap->region->entities, entity) {
        if (entity->anim.spr != SPRITE_ERROR) {
            sol_sprite_set_alpha(entity->anim.spr, alpha);
        }
    }
}

extern void map_highlight_tile(const int tilex, const int tiley, const int frame) {
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    const float zoom = settings_zoom();
    const int x = tilex * (16 * zoom) - xoffset;
    const int y = tiley * (16 * zoom) - yoffset;

    sol_sprite_set_frame(tile_highlight, frame);
    sol_sprite_set_location(tile_highlight, x - zoom, y - zoom);
    sol_sprite_render(tile_highlight);
}

static void show_debug_info() {
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    const float zoom = settings_zoom();
    int x = (xoffset + mousex) / (16 * zoom);
    int y = (yoffset + mousey) / (16 * zoom);

    if (tile_highlight == SPRITE_ERROR) {
        const float zoom = settings_zoom();
        gff_palette_t* pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
        tile_highlight = sol_sprite_new(pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20088);
    }

    map_highlight_tile(x, y, 4);
    //printf("Mouse @ x = %d, y = %d, flags = 0x%x, tile_id = %d, possible tile = %d\n", x, y, cmap->region->flags[y][x], cmap->region->tiles[y][x],
            //cmap->region->tile_ids[cmap->region->flags[y][x]&0xF]);
    //printf("0x%x\n", cmap->region->flags[117][57]);
    //printf("0x%x\n", cmap->region->flags[57][117]);
    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            if (sol_region_is_block(sol_region_manager_get_current(), x, y)) {
                map_highlight_tile(y, x, 6);
            }
        }
    }
}

static void load_wall(const int id) {
    uint16_t wall_id = (cmap->region->map_id) * 100 + id - 1;
    gff_palette_t *pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + cmap->region->map_id - 1;
    uint8_t *data = gff_get_frame_rgba_palette(DSLDATA_GFF_INDEX, GFF_WALL, wall_id, 0, pal);

    if (data) {
        int w = gff_get_frame_width(DSLDATA_GFF_INDEX, GFF_WALL, wall_id, 0);
        int h = gff_get_frame_height(DSLDATA_GFF_INDEX, GFF_WALL, wall_id, 0);
        cmap->region->walls[id] = sol_sprite_create_from_data(data, w, h);
        free(data);
    }
}

static void render_walls() {
    const int zoom = settings_zoom();
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    uint32_t xpos = -xoffset;
    uint32_t ypos = -yoffset;
    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            if (cmap->region->flags[x][y] & 0x1F) {
                // We have a wall.
                uint32_t wall_id = cmap->region->flags[x][y] & 0x1F;
                if (cmap->region->walls[wall_id] == 0) {
                    load_wall(wall_id);
                }
                sol_sprite_set_location(cmap->region->walls[wall_id], xpos,
                        ypos - sol_sprite_geth(cmap->region->walls[wall_id]) + (16 * zoom));
                sol_sprite_render(cmap->region->walls[wall_id]);
            }
            xpos += zoom * 16;
        }
        xpos = -xoffset;
        ypos += zoom * 16;
    }
}

void map_render(void *data) {
    sol_background_render();

    map_render_anims();

    if (game_over != SPRITE_ERROR) {
        sol_sprite_render(game_over);
    }

    render_walls();

    if (sol_in_debug_mode()) { show_debug_info(); }
}

static entity_t* get_parent_door(entity_t *dude) {
    if (!sol_innate_is_door(dude)) { return NULL; }

    entity_t *door2 = sol_region_find_entity_by_id(sol_region_manager_get_current(), dude->ds_id + 1);
    if (door2 == NULL || (dude->ds_id % 2)) { return NULL; }

    if ((door2->mapx == dude->mapx) || (dude->mapy == door2->mapy)) {
        return door2;
    }
    return NULL;
}

void map_render_anims() {
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    entity_t *dude, *parent_door;
    animate_sprite_t *anim;
    int hflip = 0, vflip = 0;
    int amt = 0;

    sol_static_list_display(&cmap->region->statics);

    entity_list_for_each(cmap->region->entities, dude) {
        amt++;
        hflip = vflip = 0;
        //if (dude->name) { printf("-> %s, %d, %d, %d\n", dude->name, dude->mapx, dude->mapy, dude->anim.spr); }
        if (dude->anim.spr == SPRITE_ERROR) { continue; }
        anim = &(dude->anim);
        if (!anim->scmd) { continue; }
        if (anim->scmd[anim->pos].flags & SCMD_XMIRROR
            //|| (anim->entity && anim->entity->anim.flags & 0x80)) {
            || (anim->flags & 0x80)) {
            hflip = 1;
        }
        if (anim->scmd[anim->pos].flags & SCMD_YMIRROR) {
            vflip = 1;
        }
        if (dude->anim.scmd && !(dude->anim.scmd[0].flags & SCMD_LAST)) {
            //printf("1]dude->anim.spr = %d, %p, %d, %d (%d, %d)\n", dude->anim.spr, dude->anim.scmd, anim->x, anim->y, anim->w, anim->h);
            //printf("1]dude->anim.scmd_info = %d %d %d\n", dude->anim.scmd_info.gff_idx, dude->anim.scmd_info.res_id, dude->anim.scmd_info.index);
        }
        if (!(parent_door = get_parent_door(dude))) {
            sol_sprite_set_location(anim->spr,
                anim->x - xoffset, // + scmd_xoffset,
                anim->y - yoffset); // + anim->scmd->yoffset);
        } else {
            int xdiff = 0, ydiff = 0;
            if (dude->mapy == parent_door->mapy) {
                xdiff = 16 * settings_zoom() - sol_sprite_getw(anim->spr);
            } else {
                ydiff = sol_sprite_geth(anim->spr);
                if (ydiff > 32) { ydiff = 0; }
            }
            sol_sprite_set_location(anim->spr,
                anim->x - xoffset + xdiff, // + scmd_xoffset,
                anim->y - yoffset + ydiff); // + anim->scmd->yoffset);
        }
        if (dude->name) {
            //printf("->%s\n", dude->name);
            //printf("%s: %d @ (%d, %d) frame %d / %d\n", dude->name, dude->anim.spr,
                //sol_sprite_getx(anim->spr), sol_sprite_gety(anim->spr), sol_sprite_get_frame(anim->spr), sol_sprite_num_frames(anim->spr));
        }
        sol_sprite_render_flip(anim->spr, hflip, vflip);
    }
    //printf("amt = %d\n", amt);

    sol_animation_render(&(cmap->region->actions.head->ca));
}

extern void sol_map_update_active_player(const int prev) {
    if (!cmap) { return; }
    entity_t *pr = sol_player_get(prev);
    entity_t *player = sol_player_get_active();
    if (!pr || !player) { return; }
    player->mapx = pr->mapx;
    player->mapy = pr->mapy;
    sol_region_remove_entity(sol_region_manager_get_current(), pr);
    sol_region_add_entity(sol_region_manager_get_current(), player);
    player->anim.scmd = pr->anim.scmd;
    port_entity_update_scmd(player);
    sol_map_place_entity(player);
}

void sol_map_place_entity(entity_t *entity) {
    animate_sprite_t *as = &(entity->anim);
    const float zoom = settings_zoom();

    as->x = as->destx = entity->mapx * 16 * zoom;
    as->y = as->desty = entity->mapy * 16 * zoom;
    as->w = sol_sprite_getw(entity->anim.spr);
    as->h = sol_sprite_geth(entity->anim.spr);
    if (as->w > 16 * zoom) {
        //printf("width = %d\n", as->w);
        as->x = as->destx -= (as->w - 16 * zoom) / 2;
    }
    as->y = as->desty -= as->h - (16 * zoom);
}

extern void port_load_sprite(animate_sprite_t *anim, gff_palette_t *pal, const int gff_index,
                const int type, const uint32_t id, const int num_load) {
    if (!anim) { return; }

    if (anim->scmd) {
        error("port_load_sprite not implemented for sprites with a scmd!\n");
    }

    if (anim->spr == SPRITE_ERROR) {
        anim->spr = sol_sprite_new(pal, 0, 0, settings_zoom(), gff_index, type, id);

        // Now append anything else needed.
        for (int i = 1; i < num_load; i++) {
            sol_sprite_append(anim->spr, pal, 0, 0, settings_zoom(),
                gff_index, type, id + i);
        }
    }
    //printf("valid = %d\n", sprite_valid(asn->anim->spr));
}

void port_swap_enitity(int obj_id, entity_t *dude) {
    gff_palette_t *pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + cmap->region->map_id - 1;
    const int zoom = 2.0;

    sol_sprite_free(dude->anim.spr);
    dude->anim.spr =
        sol_sprite_new(pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, dude->anim.bmp_id);
}

#define CLICKABLE (0x10)
entity_t* get_entity_at_location(const uint32_t x, const uint32_t y) {
    entity_t *dude = NULL;
    const int mapx = x - sol_get_camerax();
    const int mapy = y - sol_get_cameray();
    if (!cmap) { return 0; }

    entity_list_for_each(cmap->region->entities, dude) {
        if (dude->anim.spr == SPRITE_ERROR) { continue; }

        if (sol_sprite_in_rect(dude->anim.spr, mapx, mapy)) {
            //printf("%s: %d: %d, %d (%d, %d)\n", dude->name, dude->ds_id, dude->mapx * 16, dude->mapy * 16, mapx, mapy);
            return dude;
        }
    }

    return NULL;
}

static void update_mouse_icon() {
    enum mouse_state ms = sol_mouse_get_state();
    cdude = get_entity_at_location(sol_get_camerax() + mousex, sol_get_cameray() + mousey);
    const float zoom = settings_zoom();

    //printf("update_mouse_icon!, cdude = %p, ms = %d\n", cdude, ms);

    if (ms == MOUSE_MELEE || ms == MOUSE_NO_MELEE) {
        int x = (sol_get_camerax() + mousex) / (16 * zoom);
        int y = (sol_get_cameray() + mousey) / (16 * zoom);
        if (abs(sol_player_get_active()->mapx - x) > 1 || abs(sol_player_get_active()->mapy - y) > 1) {
            sol_mouse_set_state(MOUSE_RANGE);
            ms = sol_mouse_get_state();
        }
    } else if (ms == MOUSE_RANGE || ms == MOUSE_NO_RANGE) {
        int x = (sol_get_camerax() + mousex) / (16 * zoom);
        int y = (sol_get_cameray() + mousey) / (16 * zoom);
        if (abs(sol_player_get_active()->mapx - x) <= 1 && abs(sol_player_get_active()->mapy - y) <= 1) {
            sol_mouse_set_state(MOUSE_MELEE);
            ms = sol_mouse_get_state();
        }
    }

    if (!cdude) {
        if (ms == MOUSE_MELEE) { sol_mouse_set_state(MOUSE_NO_MELEE);
        } else if (ms == MOUSE_RANGE) { sol_mouse_set_state(MOUSE_NO_RANGE);
        } else if (ms == MOUSE_TALK) { sol_mouse_set_state(MOUSE_NO_TALK);
        } else if (ms == MOUSE_POWER) {
            switch(power_get_target_type(sol_mouse_get_power())) {
                case TARGET_ALLY:
                case TARGET_ENEMY:
                case TARGET_MULTI:
                    sol_mouse_set_state(MOUSE_NO_POWER);
                    break;
                default: break;
            }
        }
    } else {
        if (ms == MOUSE_NO_MELEE) { sol_mouse_set_state(MOUSE_MELEE);
        } else if (ms == MOUSE_NO_RANGE) { sol_mouse_set_state(MOUSE_RANGE);
        } else if (ms == MOUSE_NO_TALK) { sol_mouse_set_state(MOUSE_TALK);
        } else if (ms == MOUSE_NO_POWER) { sol_mouse_set_state(MOUSE_POWER);
        }
    }
}

int map_handle_mouse(const uint32_t x, const uint32_t y) {
    if (!cmap) { return 0; }

    mousex = x;
    mousey = y;

    update_mouse_icon();

    dude_t *dude = get_entity_at_location(sol_get_camerax() + mousex, sol_get_cameray() + mousey);
    if (sol_in_debug_mode() && dude) {
        printf("%d: %d %d\n", dude->ds_id, dude->anim.x, dude->anim.y);
        sol_entity_debug(dude);
    }

    return 1; // map always intercepts the mouse...
}

// User right clicks for the next mouse pointer type;
static void mouse_cycle() {
    enum mouse_state ms = sol_mouse_get_state();

    if (ms == MOUSE_POINTER || ms == MOUSE_NO_POINTER) {
        sol_mouse_set_state(MOUSE_MELEE);
    } else if (ms == MOUSE_MELEE || ms == MOUSE_NO_MELEE || ms == MOUSE_RANGE || ms == MOUSE_NO_RANGE) {
        sol_mouse_set_state(MOUSE_TALK);
    } else if (ms == MOUSE_NO_TALK || ms == MOUSE_TALK) {
        sol_mouse_set_state((sol_mouse_get_item()) ? MOUSE_ITEM : MOUSE_POINTER);
    } else if (ms == MOUSE_ITEM || ms == MOUSE_POWER || ms == MOUSE_NO_POWER) {
        sol_mouse_set_state(MOUSE_POINTER);
    } else {
        error ("unable to cycle from mouse state: %d\n", ms);
    }

    update_mouse_icon();
}

int map_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    enum mouse_state ms = sol_mouse_get_state();
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    const float zoom = settings_zoom();
    int tilex = (xoffset + mousex) / (16 * zoom);
    int tiley = (yoffset + mousey) / (16 * zoom);

    if (!cmap) { return 0; }
    if (game_over != SPRITE_ERROR) {
        sol_window_clear();
        sol_window_push(&main_window, 10, 10);
        return 1;
    }

    if (ms == MOUSE_TALK && cdude) {//(dude = get_entity_at_location(x, y))) {
        //printf("HERE: %d, %d, %d\n", x, y, cdude->ds_id);
        sol_mouse_set_state(MOUSE_POINTER);
        if (sol_examine_entity(cdude) == SOL_SUCCESS) {
            sol_window_push(&examine_window, 0, 0);
        } else {
            //sol_entity_debug(cdude);
            // Do an action.
            sol_innate_action(cdude);
        }
        return 1;
    }

    if (ms == MOUSE_POWER) {
        sol_combat_activate_power(sol_mouse_get_power(), sol_player_get_active(),
            get_entity_at_location(x, y), tilex, tiley);
        sol_mouse_set_state(MOUSE_POINTER);
        return 1;
    }

    if (button == SOL_MOUSE_BUTTON_RIGHT) {
        mouse_cycle();
    }

    //printf("-> %d, %d\n", tilex, tiley);
    if (ms == MOUSE_POINTER && button == SOL_MOUSE_BUTTON_LEFT) {
        sol_region_generate_move(sol_region_manager_get_current(), sol_player_get_active(), tilex, tiley, 2);
    }
    //printf("No dude there bruh.\n");

    return 1; // map always intercepts the mouse...
}

extern void port_load_item(item_t *item) {
    //warn("Need to load item %d.\n", item->ds_id);
    if (!item) { return; }
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    item->anim.spr = sol_sprite_new(pal, 0, 0, settings_zoom(),
            OBJEX_GFF_INDEX, GFF_BMP, item->anim.bmp_id);
}

extern void sol_map_game_over() {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;
    const float zoom = settings_zoom();

    if (game_over == SPRITE_ERROR) {
        game_over = sol_sprite_new(pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 10004);
        sol_sprite_set_location(game_over, 
            settings_screen_width() / 2 - sol_sprite_getw(game_over) / 2,
            settings_screen_height() / 2 - sol_sprite_geth(game_over) / 2);
    }
}

sol_wops_t map_window = {
    .init = map_load,
    .cleanup = map_cleanup,
    .render = map_render,
    .mouse_movement = map_handle_mouse,
    .mouse_down = map_handle_mouse_down,
    .mouse_up = NULL,
    .data = NULL
};
