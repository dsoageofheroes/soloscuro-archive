/* This file is auto-generated. */
#ifndef WIZARD_MORDENKAINENS_SWORD_POWER_H
#define WIZARD_MORDENKAINENS_SWORD_POWER_H

#include <stdlib.h>
#include "../effect.h"
#include "../entity.h"
#include "../rules.h"
#include "../wizard.h"

extern int wizard_mordenkainens_sword_can_activate (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_has_wizard_slot(source->entity, power_level);
}

extern int wizard_mordenkainens_sword_pay          (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_take_wizard_slot(source->entity, power_level);
}

extern void wizard_mordenkainens_sword_apply        (power_instance_t *source, entity_t *entity) {
    if (!source || !entity) { return; }
    size_t num_dice = 0, mod = 0, damage = 0;
    uint64_t effect_type = 0;
    int caster_level = (source->entity) ? entity_get_wizard_level(source->entity)
            : item_get_wizard_level(source->item);

    num_dice = 0;
    mod = 0;
    damage = dnd2e_dice_roll(num_dice, 0) + mod;
    effect_type = 0;
}

extern int wizard_mordenkainens_sword_affect_power (power_instance_t *target) {
    if (!target) { return 0;}
    return 0; // Doesn't affect powers.
}

extern int wizard_mordenkainens_sword_update       (power_t *power, power_instance_t *source) {
    if (!power || ! source) { return 0;}
    entity_t *entity = source->entity;

    if (entity) {
        power->range = 360;
        power->aoe = 1;
    }

    return 1;
}

extern void wizard_mordenkainens_sword_setup  (power_t *power) {
    power->name                 = "MORDENKAINEN'S SWORD";
    power->description          = spin_read_description(select_by_game(0, 89, 89));
    power->range                = -99999;
    power->aoe                  = -99999;
    power->shape                = TARGET_SINGLE;
    power->cast_sound           = select_by_game(219, 219, 219);
    power->thrown_sound         = select_by_game(255, 255, 255);
    power->hit_sound            = select_by_game(110, 110, 110);
    power->actions.can_activate = wizard_mordenkainens_sword_can_activate;
    power->actions.pay          = wizard_mordenkainens_sword_pay;
    power->actions.apply        = wizard_mordenkainens_sword_apply;
    power->actions.affect_power = wizard_mordenkainens_sword_affect_power;
    powers_set_icon(power, select_by_game(21088, 21088, 21088));
    powers_set_cast(power, select_by_game(8005, 8005, 8005)); // OJFF
    powers_set_thrown(power, select_by_game(8106, 8106, 8106)); // OJFF.
    powers_set_hit(power, select_by_game(8208, 8208, 8208)); // OJFF
}

#endif