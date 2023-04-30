#include <stdlib.h>
#include "arbiter.h"
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
#include "port.h"
#include "mouse.h"
#include "examine.h"
#include "gpl-var.h"
#include "window-main.h"

#include <string.h>

#define MAX_ANIMS (256)
#define HIGHLIGHT_COUNT_MAX (16)

typedef struct map_s {
    sol_region_t *region;
} map_t;

static map_t *cmap = NULL;
static int mousex = 0, mousey = 0;
static sol_sprite_t tile_highlight = SPRITE_ERROR, game_over = SPRITE_ERROR;
static sol_entity_t *dude_highlighted = NULL;
static sol_sprite_t highlight_count;
static int map_pause = 0;

void map_free(map_t *map);
static void map_load_current_region();
static map_t *create_map();
static sol_dude_t *cdude = NULL;

static void sprite_load_animation(sol_entity_t *entity, gff_palette_t *pal);
void map_render_anims();

void map_load(const uint32_t _x, const uint32_t _y) {
    sol_region_t *reg;

    sol_region_manager_get_current(&reg);
    if (!cmap && reg) {
        cmap = create_map();
        cmap->region = reg;
    }
    if (reg) {
        map_load_current_region();
        sol_center_on_player();
    }
}

void map_cleanup() {
    map_free(cmap);
    if (tile_highlight != SPRITE_ERROR) {
        sol_status_check(sol_sprite_free(tile_highlight), "Unable to free sprite");
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

static void map_load_current_region() {
    gff_palette_t *pal = NULL;
    sol_entity_t *active;
    map_t *map = cmap;

    sol_background_load_region(map->region);
    if (map->region->map_id < 100 && map->region->map_id > 0) {
        pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + map->region->map_id - 1;
        //pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + map->region->map_id - 1;
    } else {
        pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;
    }

    sol_dude_t *dude;
    sol_entity_list_for_each(map->region->entities, dude) {
        if (dude->anim.spr == SPRITE_ERROR) {
            //printf("Need to load: %s\n", dude->name ? dude->name : "");
            sprite_load_animation(dude, pal);
        }
    }

    if (sol_player_get_active(&active) == SOL_SUCCESS) {
        if (active->anim.spr == SPRITE_ERROR) {
            sprite_load_animation(active, pal);
        }
        sol_map_place_entity(active);
    }

    cmap = map;

    //TODO: Find out what maps to which areas.
    sol_audio_play_xmi(RESOURCE_GFF_INDEX, GFF_GSEQ, 2);
}

static void sprite_load_animation(sol_entity_t *entity, gff_palette_t *pal) {
    ds1_item_t ditem;
    const float zoom = settings_zoom();
    sol_sprite_info_t info;
    sol_status_check(
        sol_sprite_new(pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, entity->anim.bmp_id, &entity->anim.spr),
        "Unable to load entity sprite.");
    if (entity->name) { // If it is a combat entity, then we need to add the combat sprites
        sol_status_check(sol_sprite_append(entity->anim.spr, pal, 0, 0, zoom,
            OBJEX_GFF_INDEX, GFF_BMP, entity->anim.bmp_id + 1),
                    "Unable to append sprite.");
    }
    entity->anim.delay = 0;
    sol_status_check(sol_sprite_get_info(entity->anim.spr, &info), "Unable to get entity sprite info.");
    entity->anim.w = info.w;
    entity->anim.h = info.h;
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
    if (sol_innate_is_door(entity) == SOL_SUCCESS) {
        printf("--------->DOOR %d, 0x%x, 0x%x, 0x%x", entity->ds_id, entity->map_flags, entity->object_flags, entity->anim.flags);
        if (ssi_item_load(&ditem, entity->ds_id)) {
            printf(", item exists.special = 0x%x", ditem.special);
        } else {
            printf("item doesn't exist");
        }
        printf("\n");
    }
    //if (sol_innate_is_door(entity) && entity->map_flags & GM_ANIMATING) {
    if (sol_innate_is_door(entity) == SOL_SUCCESS && ssi_item_load(&ditem, entity->ds_id)) {
        if (ditem.special & (SSI_DOOR_GPL | SSI_DOOR_SECRET)) {
            printf("IS_DOOR GPL: %d\n", entity->ds_id );
            //sol_trigger_noorders_entity_check(entity);
            if (info.num_frames >= 4) {
                entity->map_flags &= ~GM_ANIMATING;
                sol_sprite_set_frame(entity->anim.spr, 2);
                sol_innate_activate_door(entity);
            }
        }
        if (ditem.special & SSI_DOOR_CLOSED) {
            printf("IS DOOR and should activate: %d, map_flags = 0x%x\n", entity->ds_id, entity->map_flags);
            entity->map_flags &= ~GM_ANIMATING;
            sol_sprite_set_frame(entity->anim.spr, 2);
            sol_innate_activate_door(entity);
        }
    }
}

static map_t *create_map() {
    map_t *ret = malloc(sizeof(map_t));
    memset(ret, 0x0, sizeof(map_t));
    return ret;
}

extern sol_status_t sol_map_load_region(sol_region_t *reg) {
    if (!reg) { return SOL_NULL_ARGUMENT; }
    map_free(cmap);
    if (!cmap) { cmap = create_map(); }
    //cmap->region = region_manager_get_region(reg->map_id);
    map_load_current_region();

    // TODO: NEED TO CLEAR ALL SCREENS
    sol_window_push(&map_window, 0, 0);
    sol_window_push(&narrate_window, 0, 0);
    sol_window_push(&combat_status_window, 295, 5);
    return SOL_SUCCESS;
}

extern sol_status_t sol_map_apply_alpha(const uint8_t alpha) {
    sol_entity_t *entity = NULL;
    if (!cmap) { return SOL_NOT_INITIALIZED; }

    sol_background_apply_alpha(alpha);

    sol_entity_list_for_each(cmap->region->entities, entity) {
        if (entity->anim.spr != SPRITE_ERROR) {
            sol_sprite_set_alpha(entity->anim.spr, alpha);
        }
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_map_highlight_tile(const int tilex, const int tiley, const int frame) {
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    const float zoom = settings_zoom();
    const int x = tilex * (16 * zoom) - xoffset;
    const int y = tiley * (16 * zoom) - yoffset;

    sol_sprite_set_frame(tile_highlight, frame);
    sol_sprite_set_location(tile_highlight, x - zoom, y - zoom);
    return sol_sprite_render(tile_highlight);
}

static void show_debug_info() {
    sol_region_t *reg;
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    const float zoom = settings_zoom();
    int x = (xoffset + mousex) / (16 * zoom);
    int y = (yoffset + mousey) / (16 * zoom);

    if (tile_highlight == SPRITE_ERROR) {
        const float zoom = settings_zoom();
        gff_palette_t* pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
        sol_status_check(sol_sprite_new(pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 20088, &tile_highlight),
                "Unable to load tile highlight.");
    }

    sol_map_highlight_tile(x, y, 4);
    //printf("Mouse @ x = %d, y = %d, flags = 0x%x, tile_id = %d, possible tile = %d\n", x, y, cmap->region->flags[y][x], cmap->region->tiles[y][x],
            //cmap->region->tile_ids[cmap->region->flags[y][x]&0xF]);
    //printf("0x%x\n", cmap->region->flags[117][57]);
    //printf("0x%x\n", cmap->region->flags[57][117]);
    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            sol_region_manager_get_current(&reg);
            if (sol_region_is_block(reg, x, y) == SOL_SUCCESS) {
                sol_map_highlight_tile(y, x, 6);
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
        sol_status_check(sol_sprite_create_from_data(data, w, h, &cmap->region->walls[id]),
            "Unable to load wall sprite.");
        free(data);
    }
}

static void render_walls() {
    const int zoom = settings_zoom();
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    uint32_t xpos = -xoffset;
    uint32_t ypos = -yoffset;
    sol_sprite_info_t info;

    for (int x = 0; x < 98; x++) {
        for (int y = 0; y < 128; y++) {
            if (cmap->region->flags[x][y] & 0x1F) {
                // We have a wall.
                uint32_t wall_id = cmap->region->flags[x][y] & 0x1F;
                if (cmap->region->walls[wall_id] == 0) {
                    load_wall(wall_id);
                }
                sol_status_check(sol_sprite_get_info(cmap->region->walls[wall_id], &info), "Unable to get wall sprite info");
                sol_sprite_set_location(cmap->region->walls[wall_id], xpos,
                        ypos - info.h + (16 * zoom));
                sol_sprite_render(cmap->region->walls[wall_id]);
            }
            xpos += zoom * 16;
        }
        xpos = -xoffset;
        ypos += zoom * 16;
    }
}

void map_render(void *data) {
    enum sol_mouse_state_e ms;
    sol_background_render();

    map_render_anims();

    if (game_over != SPRITE_ERROR) {
        sol_sprite_render(game_over);
    }

    render_walls();

    if (sol_in_debug_mode() == SOL_SUCCESS) { show_debug_info(); }

    sol_mouse_get_state(&ms);
    sol_map_set_pause(
            (ms != MOUSE_POINTER)
            || (sol_examine_is_open() == SOL_SUCCESS)
            || (sol_narrate_is_open() == SOL_SUCCESS)
    );
}

static sol_entity_t* get_parent_door(sol_entity_t *dude) {
    sol_region_t *reg;
    if (sol_innate_is_door(dude) != SOL_SUCCESS) { return NULL; }

    sol_entity_t *door2;
    sol_region_manager_get_current(&reg);
    sol_region_find_entity_by_id(reg, dude->ds_id + 1, &door2);
    if (door2 == NULL || (dude->ds_id % 2)) { return NULL; }

    if ((door2->mapx == dude->mapx) || (dude->mapy == door2->mapy)) {
        return door2;
    }
    return NULL;
}

extern sol_status_t sol_map_set_pause(int pause) { map_pause = pause; return SOL_SUCCESS; }
extern sol_status_t sol_map_is_paused() { return map_pause ? SOL_SUCCESS : SOL_ACTIVE; }

void map_render_anims() {
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    sol_entity_t *dude, *parent_door;
    animate_sprite_t *anim;
    int hflip = 0, vflip = 0;
    int amt = 0;
    sol_sprite_info_t info;

    sol_static_list_display(&cmap->region->statics);

    sol_entity_list_for_each(cmap->region->entities, dude) {
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
            sol_status_check(sol_sprite_get_info(anim->spr, &info), "Unable to get anim sprite info.");
            if (dude->mapy == parent_door->mapy) {
                xdiff = 16 * settings_zoom() - info.w;
            } else {
                ydiff = info.h;
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
        //sol_sprite_render_black(anim->spr, hflip, vflip);
    }

    sol_animation_render(&(cmap->region->actions.head->ca));
}

extern sol_status_t sol_map_update_active_player(const int prev) {
    sol_entity_t *pr, *player;
    sol_region_t *reg;

    if (!cmap) { return SOL_NOT_INITIALIZED; }
    sol_player_get(prev, &pr);
    sol_player_get_active(&player);
    if (!pr || !player) { return SOL_NOT_FOUND; }
    player->mapx = pr->mapx;
    player->mapy = pr->mapy;
    sol_region_manager_get_current(&reg);
    sol_region_remove_entity(reg, pr);
    sol_region_add_entity(reg, player);
    player->anim.scmd = pr->anim.scmd;
    sol_entity_update_scmd(player);
    sol_map_place_entity(player);

    return SOL_SUCCESS;
}

extern sol_status_t sol_map_place_entity(sol_entity_t *entity) {
    if (!entity) { return SOL_NULL_ARGUMENT; }
    animate_sprite_t *as = &(entity->anim);
    const float zoom = settings_zoom();
    sol_sprite_info_t info;

    as->x = as->destx = entity->mapx * 16 * zoom;
    as->y = as->desty = entity->mapy * 16 * zoom;
    sol_status_check(sol_sprite_get_info(entity->anim.spr, &info), "Unable to get entity sprite info.");
    as->w = info.w;
    as->h = info.h;
    if (as->w > 16 * zoom) {
        //printf("width = %d\n", as->w);
        as->x = as->destx -= (as->w - 16 * zoom) / 2;
    }
    as->y = as->desty -= as->h - (16 * zoom);
    return SOL_SUCCESS;
}

extern sol_status_t sol_swap_enitity(int obj_id, sol_entity_t *dude) {
    if (!dude) { return SOL_NULL_ARGUMENT; }
    gff_palette_t *pal = open_files[DSLDATA_GFF_INDEX].pals->palettes + cmap->region->map_id - 1;
    const int zoom = 2.0;

    sol_status_check(sol_sprite_free(dude->anim.spr), "Unable to free sprite");
    sol_status_check(
        sol_sprite_new(pal, 0, 0, zoom, OBJEX_GFF_INDEX, GFF_BMP, dude->anim.bmp_id, &dude->anim.spr),
        "Unable to load spr for swap.");
    return SOL_SUCCESS;
}

#define CLICKABLE (0x10)
sol_entity_t* get_entity_at_location(const uint32_t x, const uint32_t y) {
    sol_entity_t *dude = NULL;
    const int mapx = x - sol_get_camerax();
    const int mapy = y - sol_get_cameray();
    if (!cmap) { return 0; }

    sol_entity_list_for_each(cmap->region->entities, dude) {
        if (dude->anim.spr == SPRITE_ERROR) { continue; }

        if (sol_sprite_in_rect(dude->anim.spr, mapx, mapy) == SOL_SUCCESS) {
            //printf("%s: %d: %d, %d (%d, %d)\n", dude->name, dude->ds_id, dude->mapx * 16, dude->mapy * 16, mapx, mapy);
            return dude;
        }
    }

    return NULL;
}

static void update_mouse_icon() {
    enum sol_mouse_state_e ms;
    sol_power_t *power;
    cdude = get_entity_at_location(sol_get_camerax() + mousex, sol_get_cameray() + mousey);
    const float zoom = settings_zoom();
    sol_dude_t *active;

    sol_mouse_get_state(&ms);
    //printf("update_mouse_icon!, cdude = %p, ms = %d\n", cdude, ms);

    if (ms == MOUSE_MELEE || ms == MOUSE_NO_MELEE) {
        int x = (sol_get_camerax() + mousex) / (16 * zoom);
        int y = (sol_get_cameray() + mousey) / (16 * zoom);
        sol_player_get_active(&active);
        if (abs(active->mapx - x) > 1 || abs(active->mapy - y) > 1) {
            sol_mouse_set_state(MOUSE_RANGE);
            sol_mouse_get_state(&ms);
        }
    } else if (ms == MOUSE_RANGE || ms == MOUSE_NO_RANGE) {
        int x = (sol_get_camerax() + mousex) / (16 * zoom);
        int y = (sol_get_cameray() + mousey) / (16 * zoom);
        sol_player_get_active(&active);
        if (abs(active->mapx - x) <= 1 && abs(active->mapy - y) <= 1) {
            sol_mouse_set_state(MOUSE_MELEE);
            sol_mouse_get_state(&ms);
        }
    }

    if (!cdude) {
        if (ms == MOUSE_MELEE) { sol_mouse_set_state(MOUSE_NO_MELEE);
        } else if (ms == MOUSE_RANGE) { sol_mouse_set_state(MOUSE_NO_RANGE);
        } else if (ms == MOUSE_TALK) { sol_mouse_set_state(MOUSE_NO_TALK);
        } else if (ms == MOUSE_POWER) {
            sol_mouse_get_power(&power);
            enum sol_target_shape_e e;
            sol_power_get_target_type(power, &e);
            switch(e) {
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

extern sol_status_t sol_map_clear_highlight() { dude_highlighted = NULL; return SOL_SUCCESS; }

static void handle_highlight() {
    sol_dude_t *old_dude = dude_highlighted;

    dude_highlighted = get_entity_at_location(sol_get_camerax() + mousex, sol_get_cameray() + mousey);
    if (dude_highlighted && (sol_arbiter_in_combat(dude_highlighted) == SOL_IN_COMBAT)) {
        highlight_count++;
        if (highlight_count < HIGHLIGHT_COUNT_MAX) {
            sol_sprite_set_color_mod(dude_highlighted->anim.spr, 0, 0, 0);
        } else if (highlight_count < 2*HIGHLIGHT_COUNT_MAX - 1) {
            sol_sprite_set_color_mod(dude_highlighted->anim.spr, 255, 255, 255);
        } else {
            highlight_count = 0;
        }
    }

    if (old_dude != dude_highlighted && old_dude) {
        sol_sprite_set_color_mod(old_dude->anim.spr, 255, 255, 255);
        highlight_count = 0;
    }
}

static int map_handle_mouse(const uint32_t x, const uint32_t y) {
    if (!cmap) { return 0; }

    mousex = x;
    mousey = y;

    handle_highlight();

    update_mouse_icon();

    if (sol_in_debug_mode() == SOL_SUCCESS && dude_highlighted) {
        printf("%d: %d %d\n", dude_highlighted->ds_id, dude_highlighted->anim.x, dude_highlighted->anim.y);
        sol_entity_debug(dude_highlighted);
    }

    return 1; // map always intercepts the mouse...
}

// User right clicks for the next mouse pointer type;
static void mouse_cycle() {
    sol_item_t *item;
    enum sol_mouse_state_e ms;

    sol_mouse_get_state(&ms);
    if (ms == MOUSE_POINTER || ms == MOUSE_NO_POINTER) {
        sol_mouse_set_state(MOUSE_MELEE);
    } else if (ms == MOUSE_MELEE || ms == MOUSE_NO_MELEE || ms == MOUSE_RANGE || ms == MOUSE_NO_RANGE) {
        sol_mouse_set_state(MOUSE_TALK);
    } else if (ms == MOUSE_NO_TALK || ms == MOUSE_TALK) {
        sol_mouse_get_item(&item);
        sol_mouse_set_state((item) ? MOUSE_ITEM : MOUSE_POINTER);
    } else if (ms == MOUSE_ITEM || ms == MOUSE_POWER || ms == MOUSE_NO_POWER) {
        sol_mouse_set_state(MOUSE_POINTER);
    } else {
        error ("unable to cycle from mouse state: %d\n", ms);
    }

    update_mouse_icon();
}

static int map_handle_mouse_down(const uint32_t button, const uint32_t x, const uint32_t y) {
    enum sol_mouse_state_e ms;
    const uint32_t xoffset = sol_get_camerax();
    const uint32_t yoffset = sol_get_cameray();
    const float zoom = settings_zoom();
    int tilex = (xoffset + mousex) / (16 * zoom);
    int tiley = (yoffset + mousey) / (16 * zoom);
    sol_power_t *power;
    sol_entity_t *active;
    sol_region_t *reg;

    sol_mouse_get_state(&ms);
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
        sol_mouse_get_power(&power);
        sol_player_get_active(&active);
        sol_combat_activate_power(power, active,
            get_entity_at_location(x, y), tilex, tiley);
        sol_mouse_set_state(MOUSE_POINTER);
        return 1;
    }

    if (button == SOL_MOUSE_BUTTON_RIGHT) {
        mouse_cycle();
    }

    //printf("-> %d, %d\n", tilex, tiley);
    if (ms == MOUSE_POINTER && button == SOL_MOUSE_BUTTON_LEFT) {
        sol_player_get_active(&active);
        sol_region_manager_get_current(&reg);
        sol_region_generate_move(reg, active, tilex, tiley, 2);
    }
    //printf("No dude there bruh.\n");

    return 1; // map always intercepts the mouse...
}

extern sol_status_t sol_load_item(sol_item_t *item) {
    //warn("Need to load item %d.\n", item->ds_id);
    if (!item) { return SOL_NULL_ARGUMENT; }
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;

    sol_status_check(sol_sprite_new(pal, 0, 0, settings_zoom(),
            OBJEX_GFF_INDEX, GFF_BMP, item->anim.bmp_id, &item->anim.spr),
        "Unable to load item's spr.");
    return SOL_SUCCESS;
}

extern sol_status_t sol_map_game_over() {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;
    const float zoom = settings_zoom();
    sol_sprite_info_t info;

    if (game_over == SPRITE_ERROR) {
        sol_status_check(sol_sprite_new(pal, 0, 0, zoom, RESOURCE_GFF_INDEX, GFF_BMP, 10004, &game_over),
                "Unable to load game over sprite.");
        sol_status_check(sol_sprite_get_info(game_over, &info), "Unable to get entity sprite info.");
        sol_sprite_set_location(game_over, 
            settings_screen_width() / 2 - info.w / 2,
            settings_screen_height() / 2 - info.h / 2);
    }

    return SOL_SUCCESS;
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
