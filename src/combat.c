#include "combat.h"
#include "ds-player.h"
#include "ds-region.h"
#include "dsl.h"
#include "port.h"
#include <string.h>
#include <stdio.h>

void combat_init(combat_region_t *cr) {
    memset(cr, 0x0, sizeof(combat_region_t));
}

void combat_free(combat_region_t *rc) {
}

static scmd_t combat_move_down[] = {
    {.bmp_idx = 3, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 4, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 5, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 6, .delay = 7, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_move_up[] = {
    {.bmp_idx = 7, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 8, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 9, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 10, .delay = 7, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_move_right[] = {
    {.bmp_idx = 11, .delay = 7, .flags = 0x0, .xoffset = 9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 8, .flags = 0x0, .xoffset = 4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 8, .flags = 0x0, .xoffset = 8, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 7, .flags = SCMD_JUMP, .xoffset = 3, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_move_left[] = {
    {.bmp_idx = 11, .delay = 7, .flags = SCMD_XMIRROR, .xoffset = -9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 8, .flags = SCMD_XMIRROR, .xoffset = -4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 8, .flags = SCMD_XMIRROR, .xoffset = -8, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 7, .flags = SCMD_XMIRROR | SCMD_JUMP, .xoffset = -3, .yoffset = 0, 0, 0, 0},
};

static scmd_t player_move_down[] = {
    {.bmp_idx = 3, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 4, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 5, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 6, .delay = 7, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t player_move_up[] = {
    {.bmp_idx = 7, .delay = 7, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 8, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 9, .delay = 8, .flags = 0x0, .xoffset = 0, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 10, .delay = 7, .flags = SCMD_JUMP, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t player_move_right[] = {
    {.bmp_idx = 11, .delay = 7, .flags = 0x0, .xoffset = 9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 8, .flags = 0x0, .xoffset = 4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 8, .flags = 0x0, .xoffset = 8, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 7, .flags = SCMD_JUMP, .xoffset = 3, .yoffset = 0, 0, 0, 0},
};

static scmd_t player_move_left[] = {
    {.bmp_idx = 11, .delay = 7, .flags = SCMD_XMIRROR, .xoffset = -9, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 12, .delay = 8, .flags = SCMD_XMIRROR, .xoffset = -4, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 13, .delay = 8, .flags = SCMD_XMIRROR, .xoffset = -8, .yoffset = 0, 0, 0, 0},
    {.bmp_idx = 14, .delay = 7, .flags = SCMD_XMIRROR | SCMD_JUMP, .xoffset = -3, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_stand_down[] = {
    {.bmp_idx = 0, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_stand_up[] = {
    {.bmp_idx = 1, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_stand_right[] = {
    {.bmp_idx = 2, .delay = 0, .flags = SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t combat_stand_left[] = {
    {.bmp_idx = 2, .delay = 0, .flags = SCMD_XMIRROR | SCMD_LAST, .xoffset = 0, .yoffset = 0, 0, 0, 0},
};

static scmd_t *combat_types[] = {
    combat_stand_down,
    combat_stand_up,
    combat_stand_right,
    combat_stand_left,
    combat_move_down,
    combat_move_up,
    combat_move_right,
    combat_move_left,
    player_move_down,
    player_move_up,
    player_move_right,
    player_move_left,
};

static scmd_t* get_scmd(const int xdiff, const int ydiff) {
    if (xdiff < 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_LEFT); }
    if (xdiff > 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_RIGHT); }
    if (ydiff < 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_UP); }
    if (ydiff > 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_DOWN); }

    return combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
}

static int32_t ticks_per_game_round = 20;// For outside combat.

static int location_is_blocked(dsl_region_t *reg, const uint32_t x, const uint32_t y) {
    return dsl_region_is_block(reg, y, x + 1)
        || dsl_region_has_object(reg, x, y);
}

void combat_update(dsl_region_t *reg) {
    if (reg == NULL) { return; }
    combat_region_t *cr = &(reg->cr);
    player_pos_t* pc = ds_player_get_pos(ds_player_get_active());
    if (cr == NULL || pc == NULL) { return; }
    int xdiff, ydiff;
    int posx, posy;

    ticks_per_game_round--;
    if (ticks_per_game_round > 0) { return; }
    ticks_per_game_round = 30;

    for (int i = 0; i < cr->pos + 1; i++) {
        if (cr->hunt[i]) {
            xdiff = pc->xpos * 16 - cr->robjs[i]->mapx;
            ydiff = pc->ypos * 16 - cr->robjs[i]->mapy;
            xdiff = (xdiff < 0) ? -16 : (xdiff > 0) ? 16 : 0;
            ydiff = (ydiff < 0) ? -16 : (ydiff > 0) ? 16 : 0;
            posx = cr->robjs[i]->mapx / 16;
            posy = cr->robjs[i]->mapy / 16;

            if (location_is_blocked(reg, posx + xdiff/16, posy + ydiff/16)
                    ){
                if (!location_is_blocked(reg, posx, posy + ydiff/16)) {
                    xdiff = 0;
                } else if (!location_is_blocked(reg, posx + xdiff/16, posy)) {
                    ydiff = 0;
                } else {
                    xdiff = ydiff = 0;
                }
            }
            cr->robjs[i]->scmd = get_scmd(xdiff, ydiff);
            cr->robjs[i]->mapx += xdiff;
            cr->robjs[i]->mapy += ydiff;
        }
        if (cr->robjs[i]) {
            port_update_obj(cr->robjs[i], cr->robjs[i]->mapx, cr->robjs[i]->mapy);
        }
    }
}

scmd_t* combat_get_scmd(const enum combat_scmd_t type) {
    return combat_types[type];
}

static uint32_t find_next_pos(combat_region_t *rc) {
    size_t searched = 0;
    while (rc->combats[rc->pos].hp) {
        rc->pos = (rc->pos + 1) % MAX_COMBAT_OBJS;
        searched++;
        if (searched > MAX_COMBAT_OBJS) { return COMBAT_ERROR; }
    }
    return rc->pos;
}

ds1_combat_t* combat_get_combat( combat_region_t* cr, const uint32_t combat_id) {
    if (!cr || combat_id < 0 || combat_id >= MAX_COMBAT_OBJS) { return NULL; }
    return cr->combats + combat_id;
}

void combat_set_hunt(combat_region_t *cr, const uint32_t combat_id) {
    if (!cr || combat_id < 0 || combat_id >= MAX_COMBAT_OBJS) { return; }
    cr->hunt[combat_id] = 1;
}

// This does not force into combat mode, simply add a combat to the current region.
uint32_t combat_add(combat_region_t *rc, region_object_t *robj, ds1_combat_t *combat) {
    size_t i;
    if (!rc || !robj || !combat) { return COMBAT_ERROR; }
    if (find_next_pos(rc) == COMBAT_ERROR) { return COMBAT_ERROR; }

    memcpy(rc->combats + rc->pos, combat, sizeof(ds1_combat_t) - 16); // Don't copy the name over!
    for (i = 0; i < 17 && combat->name[i]; i++) {
        rc->combats[rc->pos].name[i] = combat->name[i];
    }
    // Force the name to be null-terminated.
    i = i >= 17 ? 16 : i;
    combat->name[i] = '\0';
    rc->robjs[rc->pos] = robj;
    return rc->pos;
}
