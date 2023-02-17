#include "effect.h"
#include "port.h"
#include "gpl.h"
#include "region-manager.h"
#include "entity-animation.h"

#include <string.h>

// TODO: Finish the Implementation!
extern sol_status_t sol_effect_apply_damage(sol_entity_t *source, sol_entity_t *target, const int32_t damage, sol_effect_type_t type) {
    sol_region_t *reg;
    sol_region_manager_get_current(&reg);
    if (type & EFFECT_MAGIC) {
        sol_entity_animation_list_add_effect(&(reg->actions), EA_MAGIC_DAMAGE, source, target, NULL, 30, damage);
        sol_entity_animation_list_add_effect(&(reg->actions), EA_DAMAGE_APPLY, source, target, NULL, 0, damage);
        return SOL_SUCCESS;
    }

    warn ("NEED TO implement effect_apply_damage!\n");
    return SOL_NOT_IMPLEMENTED;
}

extern sol_status_t sol_effect_apply_condition(sol_entity_t *source, sol_entity_t *target, uint16_t condition) {
    warn ("Need to apply condition!\n");
    return SOL_NOT_IMPLEMENTED;
}
