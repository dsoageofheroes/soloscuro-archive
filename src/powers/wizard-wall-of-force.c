/* This file is auto-generated. */
#ifndef WIZARD_WALL_OF_FORCE_POWER_H
#define WIZARD_WALL_OF_FORCE_POWER_H

#include <stdlib.h>
#include "../effect.h"
#include "../entity.h"
#include "../rules.h"
#include "../wizard.h"

extern int wizard_wall_of_force_can_activate (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_has_wizard_slot(source->entity, power_level);
}

extern int wizard_wall_of_force_pay          (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_take_wizard_slot(source->entity, power_level);
}

extern void wizard_wall_of_force_apply        (power_instance_t *source, entity_t *entity) {
    if (!source || !entity) { return; }
    size_t num_dice = 0, mod = 0, damage = 0;
    uint64_t effect_type = 0;
    int caster_level = (source->entity) ? entity_get_wizard_level(source->entity)
            : item_get_wizard_level(source->item);

    num_dice = 0;
    mod = 1 * caster_level;
    damage = dnd2e_dice_roll(num_dice, 0) + mod;
    effect_type = 0;
}

extern int wizard_wall_of_force_affect_power (power_instance_t *target) {
    if (!target) { return 0;}
    return 0; // Doesn't affect powers.
}

extern int wizard_wall_of_force_update       (power_t *power, power_instance_t *source) {
    if (!power || ! source) { return 0;}
    entity_t *entity = source->entity;

    if (entity) {
        power->range = 90;
        power->aoe = 0 + (40 * entity_get_wizard_level(entity));
    }

    return 1;
}

extern void wizard_wall_of_force_setup  (power_t *power) {
    power->name                 = "WALL OF FORCE";
    power->description          = spin_read_description(select_by_game(68, 67, 67));
    power->range                = -99999;
    power->aoe                  = -99999;
    power->shape                = TARGET_RECTANGLE;
    power->cast_sound           = select_by_game(77, 223, 223);
    power->thrown_sound         = select_by_game(72, 255, 255);
    power->hit_sound            = select_by_game(98, 10, 10);
    power->actions.can_activate = wizard_wall_of_force_can_activate;
    power->actions.pay          = wizard_wall_of_force_pay;
    power->actions.apply        = wizard_wall_of_force_apply;
    power->actions.affect_power = wizard_wall_of_force_affect_power;
    powers_set_icon(power, select_by_game(21066, 21066, 21066));
    powers_set_cast(power, select_by_game(8009, 8009, 8009)); // OJFF
    powers_set_thrown(power, select_by_game(8099, 8099, 8099)); // OJFF.
    powers_set_hit(power, select_by_game(-1, -1, -1)); // OJFF
}

#endif
