#include "combat.h"
#include "ds-player.h"
#include "ds-region.h"
#include "dsl.h"
#include "port.h"
#include "rules.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

combat_action_list_t action_list;

typedef struct combat_entry_s {
    int initiative;
    int sub_roll; // used to break ties.
    region_object_t *robj;
    ds1_combat_t *combat;
    struct combat_entry_s *next;
} combat_entry_t;

static int in_combat = 0;
static combat_entry_t *combat_turn = NULL;//, *combat_turn_taken = NULL;

const enum combat_turn_t combat_player_turn() {
    if (!in_combat) { return NO_COMBAT; }

    return NONPLAYER_TURN;
}

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

static scmd_t* get_scmd(scmd_t *current_scmd, const int xdiff, const int ydiff) {
    if (xdiff < 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_LEFT); }
    if (xdiff > 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_RIGHT); }
    if (ydiff < 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_UP); }
    if (ydiff > 0) { return combat_get_scmd(COMBAT_SCMD_MOVE_DOWN); }

    // xdiff and ydiff == 0.
    if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_LEFT)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_LEFT);
    } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_RIGHT)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_RIGHT);
    } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_UP)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_UP);
    } else if (current_scmd == combat_get_scmd(COMBAT_SCMD_MOVE_UP)) {
        return combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
    }

    return current_scmd;
}

static int32_t ticks_per_game_round = 20;// For outside combat.

static int location_is_blocked(dsl_region_t *reg, const uint32_t x, const uint32_t y) {
    return ds_region_location_blocked(reg, x, y)
        || dsl_region_is_block(reg, y, x + 1)
        || dsl_region_has_object(reg, x, y);
}

//TODO: Ignores walls, but that might be okay right now.
static int calc_distance_to_player(region_object_t *robj) {
    int xdiff = (robj->mapx - ds_player_get_pos(ds_player_get_active())->xpos);
    int ydiff = (robj->mapy - ds_player_get_pos(ds_player_get_active())->ypos);

    if (xdiff < 0) { xdiff *= -1;}
    if (ydiff < 0) { ydiff *= -1;}

    return xdiff > ydiff ? xdiff : ydiff;
}

static int enemies_alive(combat_region_t *cr) {
    for (int i = 0; i < cr->pos + 1; i++) {
        if (cr->combats[i].allegiance > 1) { // looks like > 1 is hostile.
            return 1; // need to check status as well.
        }
    }
    return 0;
}

static int initiative_is_less(combat_entry_t *n0, combat_entry_t *n1) {
    if (n0->initiative != n1->initiative) { return n0->initiative < n1->initiative; }
    return n0->sub_roll < n1->sub_roll;
}

// Add to the actual combat list.
static void add_to_combat(region_object_t *robj, ds1_combat_t *combat) {
    combat_entry_t *node = malloc(sizeof(combat_entry_t));
    node->robj = robj;
    node->combat = combat;
    node->next = combat_turn; // start up front.
    node->initiative = dnd2e_roll_initiative(&(combat->stats));
    node->sub_roll = dnd2e_roll_sub_roll();
    //printf("rolled: %d (%d)\n", node->initiative, node->sub_roll);

    // if the node is first.
    if (combat_turn == NULL || initiative_is_less(node, combat_turn)) {
        combat_turn = node;
        return;
    }

    // Not the first, so shift now.
    combat_entry_t *prev = combat_turn;
    node->next = prev->next;
    prev->next = node;
    while(node->next && !initiative_is_less(node, node->next)) {
        prev->next = node->next;
        node->next = node->next->next;
        prev->next->next = node;
        prev = prev->next;
    }
}

static void enter_combat_mode(dsl_region_t *reg) {
    gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes + 0;
    printf("Enter combat mode.\n");
    combat_region_t *cr = &(reg->cr);
    if (!enemies_alive(cr)) {
        error("Called to enter combat, but no enemies? ignoring...\n");
        return;
    }

    in_combat = 1;

    // Freeze all combats.
    for (int i = 0; i < cr->pos + 1; i++) {
        cr->robjs[i]->scmd = get_scmd(cr->robjs[i]->scmd, 0, 0);
        port_update_obj(cr->robjs[i], 0, 0);
    }

    port_enter_combat();

    // Right now players are not part of combat, so add them!
    for (int i = 0; i < MAX_PCS; i++) {
        if (ds_player_exists(i)) {
            combat_add(&(dsl_region_get_current()->cr), ds_player_get_robj(i), ds_player_get_combat(i));
            port_add_obj(ds_player_get_robj(i), pal);
        }
    }

    // Now lets make an initiative list
    for (int i = 0; i < cr->pos + 1; i++) {
        add_to_combat(cr->robjs[i], cr->combats + i);
    }

    // print to test:
    combat_entry_t *rover = combat_turn;
    while(rover) {
        printf("initiative: %d (%d)\n", rover->initiative, rover->sub_roll);
        rover = rover->next;
    }
}

static void do_combat_rounds(ds_region_t *reg) {
    //Need to start combat rounds.
}

void combat_update(ds_region_t *reg) {
    if (reg == NULL) { return; }
    combat_region_t *cr = &(reg->cr);
    player_pos_t* pc = ds_player_get_pos(ds_player_get_active());
    if (cr == NULL || pc == NULL) { return; }
    int xdiff, ydiff;
    int posx, posy;

    ticks_per_game_round--;
    if (ticks_per_game_round > 0) { return; }
    ticks_per_game_round = 30;

    if (in_combat) {
        for (int i = 0; i < cr->pos + 1; i++) {
            port_update_obj(cr->robjs[i], 0, 0);
        }
        do_combat_rounds(reg);
        return;
    }

    for (int i = 0; i < cr->pos + 1; i++) {
        if (cr->hunt[i]) {
            xdiff = pc->xpos - cr->robjs[i]->mapx;
            ydiff = pc->ypos - cr->robjs[i]->mapy;
            xdiff = (xdiff < 0) ? -1 : (xdiff > 0) ? 1 : 0;
            ydiff = (ydiff < 0) ? -1 : (ydiff > 0) ? 1 : 0;
            posx = cr->robjs[i]->mapx;
            posy = cr->robjs[i]->mapy;

            //printf("pos = (%d, %d)\n", posx, posy);

            if (location_is_blocked(reg, posx + xdiff, posy + ydiff)
                    ){
                if (!location_is_blocked(reg, posx, posy + ydiff)) {
                    xdiff = 0;
                } else if (!location_is_blocked(reg, posx + xdiff, posy)) {
                    ydiff = 0;
                } else {
                    xdiff = ydiff = 0;
                }
            }
            cr->robjs[i]->scmd = get_scmd(cr->robjs[i]->scmd, xdiff, ydiff);
            if (calc_distance_to_player(cr->robjs[i]) < 5) {
                enter_combat_mode(reg);
                return;
            }
            port_update_obj(cr->robjs[i], xdiff, ydiff);
        } else {
            if (cr->robjs[i]) {
                port_update_obj(cr->robjs[i], 0, 0);
            }
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
    //printf("Added %s\n", combat->name);
    rc->robjs[rc->pos] = robj;
    return rc->pos;
}
