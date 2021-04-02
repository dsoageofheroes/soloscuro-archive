/* This file is auto-generated. */
#ifndef WIZARD_FLAMING_ARROW_POWER_H
#define WIZARD_FLAMING_ARROW_POWER_H

#include <stdlib.h>
#include "../effect.h"
#include "../entity.h"
#include "../rules.h"
#include "../wizard.h"

static int wizard_flaming_arrow_can_activate (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_has_wizard_slot(source->entity, power_level);
}

static int wizard_flaming_arrow_pay          (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_take_wizard_slot(source->entity, power_level);
}

static void wizard_flaming_arrow_apply        (power_instance_t *source, entity_t *entity) {
    if (!source || !entity) { return; }
    size_t num_dice = 0, mod = 0, damage = 0;
    uint64_t effect_type = 0;
    int caster_level = (source->entity) ? entity_get_wizard_level(source->entity)
            : item_get_wizard_level(source->item);

    num_dice = (0 * ((5 + caster_level) / 5) + 0);
    mod = 0;
    damage = dnd2e_dice_roll(num_dice, 6) + mod;
    effect_type = 0 | EFFECT_FIRE | EFFECT_POINTED | EFFECT_MAGIC;
    effect_apply_damage(entity, damage, effect_type);
}

static int wizard_flaming_arrow_affect_power (power_instance_t *target) {
    if (!target) { return 0;}
    return 0; // Doesn't affect powers.
}

static int wizard_flaming_arrow_update       (power_t *power, power_instance_t *source) {
    if (!power || ! source) { return 0;}
    entity_t *entity = source->entity;

    if (entity) {
        power->range = 90 + (10 * entity_get_wizard_level(entity));
        power->aoe = 1;
    }

    return 1;
}

extern void wizard_flaming_arrow_setup  (power_t *power) {
    power->name                 = "FLAMING ARROW";
    power->description          = spin_read_description(select_by_game(29, 29, 29));
    power->range                = -99999;
    power->aoe                  = -99999;
    power->level                = 3;
    power->shape                = TARGET_ENEMY;
    power->cast_sound           = select_by_game(81, 109, 109);
    power->thrown_sound         = select_by_game(73, 255, 255);
    power->hit_sound            = select_by_game(97, 255, 255);
    power->actions.can_activate = wizard_flaming_arrow_can_activate;
    power->actions.pay          = wizard_flaming_arrow_pay;
    power->actions.apply        = wizard_flaming_arrow_apply;
    power->actions.affect_power = wizard_flaming_arrow_affect_power;
    powers_set_icon(power, select_by_game(21028, 21028, 21028));
    powers_set_cast(power, select_by_game(8005, 8005, 8005)); // OJFF
    powers_set_thrown(power, select_by_game(8113, 8106, 8106)); // OJFF.
    powers_set_hit(power, select_by_game(8208, 8208, 8208)); // OJFF
}

#endif
