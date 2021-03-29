/* This file is auto-generated. */
#ifndef WIZARD_ICE_STORM_POWER_H
#define WIZARD_ICE_STORM_POWER_H

#include <stdlib.h>
#include "../effect.h"
#include "../entity.h"
#include "../rules.h"
#include "../wizard.h"

extern int wizard_ice_storm_can_activate (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_has_wizard_slot(source->entity, power_level);
}

extern int wizard_ice_storm_pay          (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_take_wizard_slot(source->entity, power_level);
}

extern void wizard_ice_storm_apply        (power_instance_t *source, entity_t *entity) {
    if (!source || !entity) { return; }
    size_t num_dice = 0, mod = 0, damage = 0;
    uint64_t effect_type = 0;
    int caster_level = (source->entity) ? entity_get_wizard_level(source->entity)
            : item_get_wizard_level(source->item);

    num_dice = 3;
    mod = 1 * caster_level;
    damage = dnd2e_dice_roll(num_dice, 10) + mod;
    effect_type = 0 | EFFECT_COLD | EFFECT_MAGIC;
    effect_apply_damage(entity, damage, effect_type);
}

extern int wizard_ice_storm_affect_power (power_instance_t *target) {
    if (!target) { return 0;}
    return 0; // Doesn't affect powers.
}

extern int wizard_ice_storm_update       (power_t *power, power_instance_t *source) {
    if (!power || ! source) { return 0;}
    entity_t *entity = source->entity;

    if (entity) {
        power->range = 0 + (20 * entity_get_wizard_level(entity));
        power->aoe = 80;
    }

    return 1;
}

extern void wizard_ice_storm_setup  (power_t *power) {
    power->name                 = "ICE STORM";
    power->description          = spin_read_description(select_by_game(46, 44, 44));
    power->range                = -99999;
    power->aoe                  = -99999;
    power->shape                = TARGET_SINGLE;
    power->cast_sound           = select_by_game(77, 109, 109);
    power->thrown_sound         = select_by_game(71, 255, 255);
    power->hit_sound            = select_by_game(97, 110, 110);
    power->actions.can_activate = wizard_ice_storm_can_activate;
    power->actions.pay          = wizard_ice_storm_pay;
    power->actions.apply        = wizard_ice_storm_apply;
    power->actions.affect_power = wizard_ice_storm_affect_power;
    powers_set_icon(power, select_by_game(21043, 21043, 21043));
    powers_set_cast(power, select_by_game(8304, 8304, 8304)); // OJFF
    powers_set_thrown(power, select_by_game(8099, 8099, 8099)); // OJFF.
    powers_set_hit(power, select_by_game(-1, -1, -1)); // OJFF
}

#endif
