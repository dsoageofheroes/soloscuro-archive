#ifndef PSIONIC_H
#define PSIONIC_H

#include <stdint.h>

typedef struct psi_abilities_s {
    unsigned int psychokinesis    : 1;
    unsigned int psychometabolism : 1;
    unsigned int telepathy        : 1;
    unsigned int clairsentient    : 1; // future
    unsigned int psychoportive    : 1; // future
    unsigned int metapsionic      : 1; // future
    unsigned int detonate              : 1;
    unsigned int disintegrate          : 1;
    unsigned int ballistic_attack      : 1;
    unsigned int control_body          : 1;
    unsigned int iniertial_barrier     : 1;
    unsigned int annimal_affinity      : 1;
    unsigned int energy_containment    : 1;
    unsigned int life_draining         : 1;
    unsigned int absorb_disease        : 1;
    unsigned int adrenalin_control     : 1;
    unsigned int biofeedback           : 1;
    unsigned int body_weaponry         : 1;
    unsigned int cell_adjustment       : 1;
    unsigned int displacement          : 1;
    unsigned int enhanced_strength     : 1;
    unsigned int flesh_armor           : 1;
    unsigned int graft_weapon          : 1;
    unsigned int lend_health           : 1;
    unsigned int share_strength        : 1;
    unsigned int domination            : 1;
    unsigned int mas_domination        : 1;
    unsigned int psycic_crush          : 1;
    unsigned int superior_invisibility : 1;
    unsigned int tower_of_iron_will    : 1;
    unsigned int ego_whip              : 1;
} psi_abilities_t;

typedef struct psin_s {
    uint8_t types[7];
} psin_t;

typedef struct psionic_list_s {
    uint8_t psionics[34];
} psionic_list_t;

#endif
