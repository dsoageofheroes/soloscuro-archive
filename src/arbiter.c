// The arbiter decides what hits and moves.
// The idea is the network version will query the server, while the local version does the calculations.
#include "arbiter.h"
#include "port.h"
#include "map.h"
#include "region-manager.h"
#include "combat.h"
#include "combat-region.h"
#include "entity-list.h"
#include "gpl.h"
#include "gpl-state.h"
#include "player.h"
#include "rules.h"
#include "trigger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For now one combat region per region_t, later add more.
static sol_combat_region_t combat_regions[MAX_REGIONS];
static char            region_in_combat[MAX_REGIONS] = {0};

extern sol_status_t sol_arbiter_hits(sol_entity_animation_node_t *ea) {
    if (!ea) { return SOL_NULL_ARGUMENT; }
    return SOL_SUCCESS;
}

static int get_dist(sol_entity_t *entity, const uint16_t x, const uint16_t y) {
    int xdiff = abs(entity->mapx - x);
    int ydiff = abs(entity->mapy - y);

    return (xdiff > ydiff) ? xdiff : ydiff;
}

extern sol_status_t sol_arbiter_combat_region(sol_region_t *reg, sol_combat_region_t **cr) {
    if (!reg) {return SOL_NULL_ARGUMENT; }
    if (reg->map_id < 0 || reg->map_id >= MAX_REGIONS) { return SOL_ILLEGAL_ARGUMENT; }

    if (!region_in_combat[reg->map_id]) {
        *cr = NULL;
    } else {
        *cr = combat_regions + reg->map_id;
    }

    return SOL_SUCCESS;
}

static void place_combatants(sol_combat_region_t *cr) {
    if (!cr) { return; }
    sol_dude_t *entity = NULL;

    sol_entity_list_for_each((&(cr->combatants)), entity) {
        memset(&entity->stats.combat, 0x0, sizeof(sol_combat_round_stats_t));
        sol_dnd2e_roll_initiative(entity);
        entity->stats.combat.move = dnd2e_get_move(entity);
        sol_entity_list_add_by_init((&cr->round.entities), entity, NULL);
    }
}

extern sol_status_t sol_arbiter_next_round(sol_combat_region_t* cr) {
    if (!cr) { return SOL_NULL_ARGUMENT; }
    int16_t gn;
    sol_status_t status;

    cr->round.num++;
    // Increment GPL's game time.
    if ((status = sol_gpl_get_gname(GNAME_TIME, &gn)) != SOL_SUCCESS) {
        return status;
    }
    sol_gpl_set_gname(GNAME_TIME, gn + 60);
    place_combatants(cr);
    return SOL_SUCCESS;
}

extern sol_status_t sol_arbiter_enter_combat(sol_region_t *reg, const uint16_t x, const uint16_t y) {
    const int        dist = 10; // distance of the sphere;
    sol_dude_t          *enemy = NULL;
    sol_combat_region_t *cr = NULL;
    sol_status_t     status = SOL_UNKNOWN_ERROR;

    // validate args and confirm we can enter combat.
    if (!reg) { return SOL_NULL_ARGUMENT; }
    if (reg->map_id < 0 || reg->map_id >= MAX_REGIONS) {
        error("reg->map_id = %d, which is out of range!\n", reg->map_id);
        return SOL_ILLEGAL_ARGUMENT;
    }
    if (region_in_combat[reg->map_id]) { return 0; }
    cr = combat_regions + reg->map_id;

    sol_combat_region_init(cr);

    for (int i = 0; i < 4; i++) {
        sol_dude_t *next_player, *active;
        sol_player_get(i, &next_player);
        sol_player_get_active(&active);
        if (next_player && next_player != active && next_player->name) { // next_player exists.
            //printf("Adding %s (%d, %d)\n", next_player->name, next_player->mapx, next_player->mapy);
            // force a load of player's sprites.
            error("Not adding player %s\n", next_player->name);
            /*
            sol_player_get_sprite(i);
            sol_region_move_to_nearest(sol_region_manager_get_current(), next_player);
            sol_region_add_entity(sol_region_manager_get_current(), next_player);
            sol_map_place_entity(next_player);
            */
        }
    }

    sol_entity_list_for_each(reg->entities, enemy) {
        if (enemy->name && get_dist(enemy, x, y) <= dist) {
            sol_entity_list_add(&(cr->combatants), enemy, NULL);
        }
    }

    if (sol_combat_check_if_over(cr) == SOL_SUCCESS) {
        sol_combat_clear(cr);
        return region_in_combat[reg->map_id] = 0;
    }

    region_in_combat[reg->map_id] = 1;

    status = sol_arbiter_next_round(cr);
    sol_status_check(status, "Unable to start next round!\n");

    return region_in_combat[reg->map_id] ? SOL_SUCCESS : SOL_NOT_IN_COMBAT;
}

extern sol_status_t sol_arbiter_combat_check(sol_region_t* reg) {
    if (!reg || reg->map_id < 0 || reg->map_id >= MAX_REGIONS) { return SOL_NULL_ARGUMENT; }

    if (sol_combat_check_if_over(combat_regions + reg->map_id) == SOL_SUCCESS) {
        region_in_combat[reg->map_id] = 0;
        sol_trigger_noorders_event();
    }

    return SOL_SUCCESS;
}

// DS Engine: Attacks usually based on weapons. If none, then bare hand.
//            Monster's base weapon are considered plus for
//            level 0-4: regular
//            level 5-6: +1
//            level 6-7: +2
//            level 8-9: +3
//            level 10+: +4
// Monster data should be in the entity.
// For Now, 1d6, always hits. Need to add thac0 calculation.
extern sol_status_t sol_arbiter_entity_attack(sol_entity_t *source, sol_entity_t *target, int round, enum sol_entity_action_e action, sol_attack_t *attack) {
    sol_status_t        status = SOL_UNKNOWN_ERROR;
    static sol_attack_t error = { -2, 0 };
    sol_combat_region_t    *cr = NULL;
    sol_region_t       *reg;
    sol_entity_t       *cur;

    sol_region_manager_get_current(&reg);
    status = sol_arbiter_combat_region(reg, &cr);
    sol_status_check(status, "Unable to get combat region!");
     
    // For now just use local until MMO is up.
    if (sol_combat_get_current(cr, &cur) != SOL_SUCCESS || (cur != source && source->combat_status != EA_GUARD)) {
        *attack = error;
        return SOL_ILLEGAL_ATTACK;
    }

    switch (action) {
        case EA_GUARD:   if (sol_dnd2e_can_melee_again(source, source->stats.combat.attack_num ,round) != SOL_SUCCESS) {
                             *attack = error;
                             return SOL_ILLEGAL_ARGUMENT;
                         }
        case EA_MELEE:   *attack = sol_dnd2e_melee_attack(source, target, round);
                         return SOL_ILLEGAL_ATTACK;
        case EA_MISSILE: *attack = sol_dnd2e_range_attack(source, target, round);
                         return SOL_ILLEGAL_ATTACK;
        default:
            warn("unknown action %d\n", action);
            error.damage = -2;
            *attack = error;
    }

    return SOL_SUCCESS;
}

extern sol_status_t sol_arbiter_in_combat(sol_dude_t *dude) {
    for (size_t i = 0; i < MAX_REGIONS; i++) {
        if (sol_entity_list_find(&(combat_regions[i].combatants), dude, NULL) == SOL_SUCCESS) {
            return SOL_IN_COMBAT;
        }
    }

    return SOL_NOT_IN_COMBAT;
}
