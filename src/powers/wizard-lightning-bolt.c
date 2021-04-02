/* This file is auto-generated. */
#ifndef WIZARD_LIGHTNING_BOLT_POWER_H
#define WIZARD_LIGHTNING_BOLT_POWER_H

#include <stdlib.h>
#include "../effect.h"
#include "../entity.h"
#include "../rules.h"
#include "../wizard.h"

static int wizard_lightning_bolt_can_activate (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_has_wizard_slot(source->entity, power_level);
}

static int wizard_lightning_bolt_pay          (power_instance_t *source, const int16_t power_level) {
    if (!source || !source->entity) { return 0;}
    return entity_take_wizard_slot(source->entity, power_level);
}

static void wizard_lightning_bolt_apply        (power_instance_t *source, entity_t *entity) {
    if (!source || !entity) { return; }
    size_t num_dice = 0, mod = 0, damage = 0;
    uint64_t effect_type = 0;
    int caster_level = (source->entity) ? entity_get_wizard_level(source->entity)
            : item_get_wizard_level(source->item);

    num_dice = (1 * caster_level + 0);
    mod = 1 * caster_level;
    damage = dnd2e_dice_roll(num_dice, 6) + mod;
    effect_type = 0 | EFFECT_ELECTRIC | EFFECT_MAGIC;
    effect_apply_damage(entity, damage, effect_type);
}

static int wizard_lightning_bolt_affect_power (power_instance_t *target) {
    if (!target) { return 0;}
    return 0; // Doesn't affect powers.
}

static int wizard_lightning_bolt_update       (power_t *power, power_instance_t *source) {
    if (!power || ! source) { return 0;}
    entity_t *entity = source->entity;

    if (entity) {
        power->range = 400 + (40 * entity_get_wizard_level(entity));
        power->aoe = 40;
    }

    return 1;
}

extern void wizard_lightning_bolt_setup  (power_t *power) {
    power->name                 = "LIGHTNING BOLT";
    power->description          = spin_read_description(select_by_game(33, 33, 33));
    power->range                = -99999;
    power->aoe                  = -99999;
    power->level                = 3;
    power->shape                = TARGET_SINGLE;
    power->cast_sound           = select_by_game(81, 4, 4);
    power->thrown_sound         = select_by_game(73, 255, 255);
    power->hit_sound            = select_by_game(97, 109, 109);
    power->actions.can_activate = wizard_lightning_bolt_can_activate;
    power->actions.pay          = wizard_lightning_bolt_pay;
    power->actions.apply        = wizard_lightning_bolt_apply;
    power->actions.affect_power = wizard_lightning_bolt_affect_power;
    powers_set_icon(power, select_by_game(21032, 21032, 21032));
    powers_set_cast(power, select_by_game(8006, 8006, 8006)); // OJFF
    powers_set_thrown(power, select_by_game(8105, 8108, 8108)); // OJFF.
    powers_set_hit(power, select_by_game(8210, 8210, 8210)); // OJFF
}

#endif
