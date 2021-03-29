#include "../wizard.h"

static int wizard_burning_hands_can_activate (struct power_instance_s *source) {
    return 0;
}

static int wizard_burning_hands_pay          (struct power_instance_s *source) {
    return 0;
}

static int wizard_burning_hands_apply        (struct power_instance_s *source, entity_t *target) {
    return 0;
}

static int wizard_burning_hands_still_active (struct power_instance_s *source, const int rounds_past) {
    return 0;
}

static int wizard_burning_hands_affect_power (struct power_instance_s *target) {
    return 0;
}

extern void wizard_burning_hands_attach (power_t *spell) {
    spell->name                 = "Burning Hands";
    spell->description          = spells_read_description(2);
    spell->range                = 0;
    spell->aoe                  = 0;
    spell->shape                = TARGET_SINGLE;
    spell->actions.can_activate = wizard_burning_hands_can_activate;
    spell->actions.pay          = wizard_burning_hands_pay;
    spell->actions.apply        = wizard_burning_hands_apply;
    spell->actions.still_active = wizard_burning_hands_still_active;
    spell->actions.affect_power = wizard_burning_hands_affect_power;
    spells_set_icon(spell, 21001);
    spells_set_thrown(spell, 2315);
    spells_set_hit(spell, 2314);
}
