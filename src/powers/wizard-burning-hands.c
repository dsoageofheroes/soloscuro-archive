/* This file is auto-generated. */
#ifndef WIZARD_BURNING_HANDS_POWER_H
#define WIZARD_BURNING_HANDS_POWER_H

#include <stdlib.h>
#include "../effect.h"
#include "../entity.h"
#include "../rules.h"
#include "../wizard.h"

static int wizard_burning_hands_can_activate (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_has_wizard_slot(source->entity, power_level);
}

static int wizard_burning_hands_pay          (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_take_wizard_slot(source->entity, power_level);
}

static void wizard_burning_hands_apply        (power_instance_t *source, entity_t *entity) {
    if (!source || !entity) { return; }
    size_t num_dice = 0, mod = 0, damage = 0;
    uint64_t effect_type = 0;
    int caster_level = (source->entity) ? entity_get_wizard_level(source->entity)
            : item_get_wizard_level(source->item);

    num_dice = 1;
    mod = (0 + 2) * caster_level;
    damage = dnd2e_dice_roll(num_dice, 3) + mod;
    effect_type = 0 | EFFECT_FIRE | EFFECT_MAGIC;
    effect_apply_damage(entity, damage, effect_type);
}

static int wizard_burning_hands_affect_power (power_instance_t *target) {
    if (!target) { return 0;}
    return 0; // Doesn't affect powers.
}

static int wizard_burning_hands_update       (power_t *power, power_instance_t *source) {
    if (!power || ! source) { return 0;}
    entity_t *entity = source->entity;

    if (entity) {
        power->range = 40;
        power->aoe = 48;
    }

    return 1;
}

extern void wizard_burning_hands_setup  (power_t *power) {
    power->name                 = "BURNING HANDS";
    power->description          = spin_read_description(select_by_game(2, 2, 2));
    power->range                = -99999;
    power->aoe                  = -99999;
    power->level                = 1;
    power->shape                = TARGET_CONE;
    power->cast_sound           = select_by_game(81, 255, 255);
    power->thrown_sound         = select_by_game(73, 255, 255);
    power->hit_sound            = select_by_game(97, 109, 109);
    power->actions.can_activate = wizard_burning_hands_can_activate;
    power->actions.pay          = wizard_burning_hands_pay;
    power->actions.apply        = wizard_burning_hands_apply;
    power->actions.affect_power = wizard_burning_hands_affect_power;
    powers_set_icon(power, select_by_game(21001, 21001, 21001));
    powers_set_cast(power, select_by_game(-1, -1, -1)); // OJFF
    powers_set_thrown(power, select_by_game(8099, 8099, 8099)); // OJFF.
    powers_set_hit(power, select_by_game(-1, -1, -1)); // OJFF
}

#endif
