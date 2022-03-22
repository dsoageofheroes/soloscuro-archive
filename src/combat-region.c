#include "combat.h"
#include "combat-region.h"
#include "entity-list.h"
#include "region-manager.h"
#include "player.h"
#include <stdio.h>
#include <string.h>

extern void sol_combat_region_init(combat_region_t *cr) {
    memset(cr, 0x0, sizeof(combat_region_t));
    entity_list_init(cr->combatants);
    entity_list_init(cr->round.entities);
}

extern int sol_combat_check_if_over(combat_region_t *cr) {
    entity_t *combatant;
    if (!cr || !cr->combatants.head) { return 1; }
    int allegiance = -1;

    entity_list_for_each((&(cr->combatants)), combatant) {
        if (allegiance == -1) { allegiance = combatant->allegiance; }
        if (allegiance != combatant->allegiance) {
            return 0;
        }
    }

    return 1;
}

extern void sol_combat_clear(combat_region_t *cr) {
    if (!cr || !cr->combatants.head) { return; }
    entity_list_clear((&(cr->combatants)));
    entity_list_clear((&(cr->round.entities)));
}

extern entity_t* sol_combat_get_current(combat_region_t *cr) {
    if (!cr || !cr->round.entities.head) { return NULL; }
    return cr->round.entities.head->entity;
}

extern void sol_combat_next_combatant(combat_region_t *cr) {
    if (!cr) { return; }

    entity_list_remove_entity(&cr->round.entities, cr->round.entities.head->entity);
    if (cr->round.entities.head) {
        cr->round.entities.head->entity->combat_status = EA_NONE;
    }
}

extern int sol_combat_attempt_action(combat_region_t *cr, dude_t *dude) {
    if (!cr || cr->round.entities.head->entity != dude || !dude) { return 0; }

    if (dude->stats.combat.move <= 0) { return 0; }

    dude->stats.combat.move--;
    return 1;
}

// This is called right at the end of an animation.
extern int sol_combat_guard_check(combat_region_t *cr) {
    entity_t *dude = sol_combat_get_current(cr), *enemy = NULL;
    if (!dude) { return 0; }
      
    entity_list_for_each((&(cr->combatants)), enemy) {
        //printf("(%d, %d) vs (%d, %d)\n", dude->mapx, dude->mapy, enemy->mapx, enemy->mapy);
        if (enemy == dude) { continue; }
        if (abs(enemy->mapx - dude->mapx) <= 1 && abs(enemy->mapy - dude->mapy) <= 1) {
            if (enemy->combat_status == EA_GUARD) {
                //printf("%s should attack %s!\n", enemy->name, dude->name);
                if (sol_combat_add_attack_animation(sol_region_manager_get_current(),
                              enemy, dude, NULL, EA_GUARD) == -2) {
                    enemy->combat_status = EA_NONE;
                    //printf("out of attacks.\n");
                }

                return 1;
            }
        }
    }
  
    return 0;
}

extern entity_t* sol_combat_get_closest_enemy(combat_region_t *cr, const int x, const int y) {
    entity_t *dude, *ret = NULL;
    int       max = 9999999;

    entity_list_for_each((&(cr->combatants)), dude) {
        int dx = abs(dude->mapx - x);
        int dy = abs(dude->mapy - y);
        int tmax = dx > dy ? dx : dy;

        if (sol_player_get_slot(dude) >= 0) { continue; } 
        if (tmax == 0) { continue; }
        if (tmax < max) {
            max = tmax;
            ret = dude;
        }
    }

    return ret;
}
