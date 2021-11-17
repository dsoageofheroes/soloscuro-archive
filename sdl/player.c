#include <math.h>
#include "player.h"
#include "main.h"
#include "../src/combat.h"
#include "../src/dsl.h"
#include "../src/settings.h"
#include "../src/trigger.h"
#include "../src/entity-animation.h"
#include "../src/port.h"
#include "../src/region-manager.h"
#include "../src/player.h"
#include "../src/gameloop.h"
#include "../src/dsl-var.h"
#include "../src/gff-map.h"
#include "../src/gff-image.h"
#include "../src/gff.h"
#include "narrate.h"
#include "gfftypes.h"

enum entity_action_e last_action[MAX_PCS] = { EA_WALK_DOWN, EA_WALK_DOWN, EA_WALK_DOWN, EA_WALK_DOWN };

void player_init() {
}

static int ticks_per_move = 30;
static int count = 0;
static int direction = 0x0;

void player_update() {
    entity_t *dude = player_get_active();
    int xdiff = 0, ydiff = 0;
    const enum combat_turn_t combat_turn = combat_player_turn();
    enum entity_action_e action;

    //if (entity_animation_execute(dude)) { --count; return; }
    //entity_animation_list_execute(&(dude->actions), region_manager_get_current());
    if (--count > 0) { return; }

    //if (entity_animation_list_execute(&(dude->actions), region_manager_get_current())) {
        //count = ticks_per_move;
        //return;
    //}

    // update when we can have the player take a turn.
    if (combat_turn != NO_COMBAT) { return; }

    if (direction & PLAYER_UP)    { ydiff -= 1; }
    if (direction & PLAYER_DOWN)  { ydiff += 1; }
    if (direction & PLAYER_LEFT)  { xdiff -= 1; }
    if (direction & PLAYER_RIGHT) { xdiff += 1; }

    if (sol_player_freeze() || region_is_block(region_manager_get_current(),
                //dude->mapx + xdiff, dude->mapy + ydiff)) {
                dude->mapy + ydiff, dude->mapx + xdiff)) {
        xdiff = ydiff = 0;
    }

    if (!narrate_is_open()) {
        trigger_noorders(dude->mapx, dude->mapy);
    }
    trigger_box_check(dude->mapx, dude->mapy);
    trigger_tile_check(dude->mapx, dude->mapy);

    // We aren't moving...
    if (xdiff == 0 && ydiff == 0) {
        dude->anim.movex = dude->anim.movey = 0.0;
        dude->anim.scmd = entity_animation_face_direction(dude->anim.scmd,
            last_action[player_get_active_slot()]);
        entity_animation_list_add(&(dude->actions), EA_NONE, dude, NULL, NULL, 1);
        return;
    }

    action =
          (xdiff == 1 && ydiff == 1) ? EA_WALK_DOWNRIGHT
        : (xdiff == 1 && ydiff == -1) ? EA_WALK_UPRIGHT
        : (xdiff == -1 && ydiff == -1) ? EA_WALK_UPLEFT
        : (xdiff == -1 && ydiff == 1) ? EA_WALK_DOWNLEFT
        : (xdiff == 1) ? EA_WALK_RIGHT
        : (xdiff == -1) ? EA_WALK_LEFT
        : (ydiff == 1) ? EA_WALK_DOWN
        : (ydiff == -1) ? EA_WALK_UP
        : EA_NONE;
    last_action[player_get_active_slot()] = action;

    entity_animation_list_add_speed(&(dude->actions), action, dude, NULL, NULL, ticks_per_move, 2);
    count = ticks_per_move / 2;

    dude->mapx += xdiff;
    dude->mapy += ydiff;
    dude->anim.destx += (xdiff * 32);
    dude->anim.desty += (ydiff * 32);
    region_t *reg = region_manager_get_current();
    if (reg) {
        animation_shift_entity(reg->entities, entity_list_find(reg->entities, dude));
    }
}

void player_move(const uint8_t _direction) {
    direction |= _direction;
}

void player_unmove(const uint8_t _direction) {
    direction &= ~(_direction);
}

extern void player_condense() {
    for (int i = 0; i < MAX_PCS; i++) {
        entity_t *player = player_get(i);
        if (player != player_get_active() && player->name) {
            port_update_entity(player, -999, -999);
        }
    }
}

extern void port_player_load(const int slot) {
    sol_player_load(slot, settings_zoom());
    sol_player_load_graphics(slot);
    port_place_entity(player_get(slot));
}

void player_set_delay(const int amt) {
    if (amt < 0) { return; }

    for (int i = 0; i < 4; i++) {
        combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_DOWN)[i].delay = amt;
        combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_UP)[i].delay = amt;
        combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_LEFT)[i].delay = amt;
        combat_get_scmd(COMBAT_SCMD_PLAYER_MOVE_RIGHT)[i].delay = amt;
    }
}

void player_set_move(const int amt) {
    if (amt < 0) { return; }
    ticks_per_move = amt;
}

