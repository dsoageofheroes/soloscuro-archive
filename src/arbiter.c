// The arbiter decides what hits and moves.
// The idea is the network version will query the server, while the local version does the calucaltions.
#include "arbiter.h"
#include "region-manager.h"
#include "combat.h"
#include "entity-list.h"
#include "gpl.h"
#include <stdio.h>

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

extern int sol_arbiter_is_in_combat(sol_region_t *reg) {
    if (!reg || reg->map_id < 0 || reg->map_id >= MAX_REGIONS) { return 0; }
    return region_in_combat[reg->map_id];
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

    sol_combat_init(cr);
    entity_list_for_each(reg->entities, enemy) {
        if (enemy->name && get_dist(enemy, x, y) <= dist) {
            entity_list_add(&(cr->combatants), enemy);
        }
    }

    if (sol_combat_check_if_over(cr)) {
        sol_combat_clear(cr);
        return region_in_combat[reg->map_id] = 0;
    }

    printf("reg = %p\n", reg);
    printf("Need to start combat\n");
    // Freeze all combats.
    /*
    entity_list_for_each(reg->cr.combatants, enemy) {
        enemy->anim.scmd = entity_animation_get_scmd(enemy->anim.scmd, 0, 0, EA_NONE);
        port_update_entity(enemy, 0, 0);
    }

    for (int i = 0; i < MAX_PCS; i++) {
        if (sol_player_exists(i)) {
            entity_list_add(reg->cr.combatants, sol_player_get(i));
        }
    }

    in_combat = 1;

    entity_list_for_each(reg->cr.combatants, enemy) {
        add_to_combat(enemy);
    }
    */

    sol_combat_enter_combat();

    /*
    combat_entry_t *rover = combat_order;
    while(rover) {
        printf("%s: %p\n", rover->entity->name, rover->entity->sprite.scmd);
        rover = rover->next;
    }
    */

    return region_in_combat[reg->map_id];
}
