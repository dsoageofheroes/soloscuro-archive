#include "combat-region.h"
#include "entity-list.h"
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
}

extern int sol_combat_attempt_action(combat_region_t *cr, dude_t *dude) {
    if (!cr || cr->round.entities.head->entity != dude || !dude) { return 0; }

    if (dude->stats.combat.move <= 0) { return 0; }

    dude->stats.combat.move--;
    return 1;
}
