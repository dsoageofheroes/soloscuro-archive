// The arbiter decides what hits and moves.
// The idea is the network version will query the server, while the local version does the calucaltions.
#include "arbiter.h"
#include "region-manager.h"
#include "combat.h"
#include "combat-region.h"
#include "entity-list.h"
#include "gpl.h"
#include "rules.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// For now one combat region per region_t, later add more.
static combat_region_t combat_regions[MAX_REGIONS];
static char            region_in_combat[MAX_REGIONS] = {0};

extern int sol_arbiter_hits(entity_animation_node_t *ea) {
    if (!ea) { return 0; }
    return 1;
}

static int get_dist(entity_t *entity, const uint16_t x, const uint16_t y) {
    int xdiff = abs(entity->mapx - x);
    int ydiff = abs(entity->mapy - y);

    return (xdiff > ydiff) ? xdiff : ydiff;
}

extern combat_region_t* sol_arbiter_combat_region(sol_region_t *reg) {
    if (!reg || reg->map_id < 0 || reg->map_id >= MAX_REGIONS) { return NULL; }
    if (!region_in_combat[reg->map_id]) { return NULL; }

    return combat_regions + reg->map_id;
}

static void place_combatants(combat_region_t *cr) {
    if (!cr) { return; }
    dude_t *entity = NULL;

    entity_list_for_each((&(cr->combatants)), entity) {
        memset(&entity->stats.combat, 0x0, sizeof(combat_round_stats_t));
        entity->stats.combat.initiative = dnd2e_roll_initiative(entity);
        entity->stats.combat.move = dnd2e_get_move(entity);
        entity_list_add_by_init((&cr->round.entities), entity);
    }
}

extern void sol_arbiter_next_round(combat_region_t* cr) {
    if (!cr) { return; }
    cr->round.num++;
    place_combatants(cr);
}

extern int sol_arbiter_enter_combat(sol_region_t *reg, const uint16_t x, const uint16_t y) {
    const int dist = 10; // distance of the sphere;
    dude_t *enemy = NULL;
    combat_region_t *cr;

    // validate args and confirm we can enter combat.
    if (!reg) { return 0; }
    if (reg->map_id < 0 || reg->map_id >= MAX_REGIONS) {
        error("reg->map_id = %d, which is out of range!\n", reg->map_id);
        return 0;
    }
    if (region_in_combat[reg->map_id]) { return 0; }
    cr = combat_regions + reg->map_id;

    sol_combat_region_init(cr);
    entity_list_for_each(reg->entities, enemy) {
        if (enemy->name && get_dist(enemy, x, y) <= dist) {
            entity_list_add(&(cr->combatants), enemy);
        }
    }

    if (sol_combat_check_if_over(cr)) {
        sol_combat_clear(cr);
        return region_in_combat[reg->map_id] = 0;
    }

    region_in_combat[reg->map_id] = 1;

    sol_arbiter_next_round(cr);

    return region_in_combat[reg->map_id];
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
extern sol_attack_t sol_arbiter_enemy_melee_attack(entity_t *source, entity_t *target, int round) {
    return dnd2e_melee_attack(source, target, round);
    /*
    //int16_t dnd2e_melee_attack(entity_t *source, entity_t *target, const int attack_num, const int round) {
    entity_animation_add(EA_MELEE, source, NULL, NULL, 0);
    //printf("amt = %d!\n", amt);
    if (amt > 0) {
        entity_animation_add(EA_RED_DAMAGE, source, target, NULL, amt);
        entity_animation_add(EA_DAMAGE_APPLY, source, target, NULL, amt);
    }
    */
}
