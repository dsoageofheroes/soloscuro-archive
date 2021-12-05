#ifndef WIZARD_H
#define WIZARD_H

#include <stdint.h>
#include "powers.h"
#include "psionic.h"
#include "entity.h"
#include "item.h"

#define MAX_SPELLS (8*138)
#define MAX_PSIONICS (34)

enum {
    PSIONIC_PSYCHOKINETIC,
    PSIONIC_PSYCHOMETABOLISM,
    PSIONIC_TELEPATH,
    SPHERE_AIR,
    SPHERE_EARTH,
    SPHERE_FIRE,
    SPHERE_WATER,
    PSIN_MAX,
};

enum {
    WIZ_ARMOR,
    WIZ_BURNING_HANDS,
    WIZ_CHARM_PERSON,
    WIZ_CHILL_TOUCH,
    WIZ_COLOR_SPRAY,
    WIZ_ENLARGE,
    WIZ_GAZE_REFLECTION,
    WIZ_GREASE,
    WIZ_MAGIC_MISSILE,
    WIZ_SHIELD,
    WIZ_SHOCKING_GRASP,
    WIZ_WALL_OF_FOG,
    WIZ_BLUR,
    WIZ_DETECT_INVISIBLE,
    WIZ_FLAMING_SPHERE,
    WIZ_FOG_CLOUD,
    WIZ_GLITTER_DUST,
    WIZ_INVISIBILITY,
    WIZ_ACID_ARROW,
    WIZ_MIRROR_IMAGE,
    WIZ_PROTECTION_FROM_PARALYSIS,
    WIZ_SCARE,
    WIZ_STINKING_CLOUD,
    WIZ_STRENGTH,
    WIZ_WEB,
    WIZ_BLINK,
    WIZ_DISPEL_MAGIC,
    WIZ_FIREBALL,
    WIZ_FLAMING_ARROW,
    WIZ_HASTE,
    WIZ_HOLD_PERSON,
    WIZ_HOLD_UNDEAD,
    WIZ_LIGHTNING_BOLT,
    WIZ_MINUTE_METEORS,
    WIZ_MINOR_MALISON,
    WIZ_PROTECTION_FROM_MISSILES,
    WIZ_SLOW,
    WIZ_SPIRIT_ARMOR,
    WIZ_VAMPIRIC_TOUCH,
    WIZ_MONSTER_SUMMONING_I,
    WIZ_CHARM_MONSTER,
    WIZ_CONFUSION,
    WIZ_FEAR,
    WIZ_FIRE_SHIELD,
    WIZ_ICE_STORM,
    WIZ_IMPROVED_INVISIBILITY,
    WIZ_M_GLOBE_OF_INVULNERABILITY,
    WIZ_MINOR_SPELL_TURNING,
    WIZ_OTILUKES_RESILIENT_SPHERE,
    WIZ_PSIONIC_DAMPENER,
    WIZ_RAINBOW_PATTERN,
    WIZ_SOLID_FOG,
    WIZ_SPIDER_STRAND,
    WIZ_STONE_SKIN,
    WIZ_PEBBLE_TO_BOULDER,
    WIZ_WALL_OF_FIRE,
    WIZ_WALL_OF_ICE,
    WIZ_MONSTER_SUMMONING_II,
    WIZ_EVARDS_BLACK_TENTACLES,
    WIZ_CHAOS,
    WIZ_CLOUD_KILL,
    WIZ_CONE_OF_COLD,
    WIZ_CONJURE_ELEMENTAL,
    WIZ_DISMISSAL,
    WIZ_DOMINATE,
    WIZ_FEEBLE_MIND,
    WIZ_HOLD_MONSTER,
    WIZ_LOWER_RESISTANCE_TO_MAGIC,
    WIZ_SUMMON_SHADOW,
    WIZ_WALL_OF_FORCE,
    WIZ_WALL_OF_STONE,
    WIZ_MONSTER_SUMMONING_III,
    WIZ_ANTI_MAGIC_SHELL,
    WIZ_CHAIN_LIGHTNING,
    WIZ_DEATH_FOG,
    WIZ_DEATH_SPELL,
    WIZ_DISINTEGRATE,
    WIZ_GLOBE_OF_INVULNERABILITY,
    WIZ_IMPROVED_HASTE,
    WIZ_IMPROVED_SLOW,
    WIZ_MONSTER_SUMMONING_IV,
    WIZ_REINCARNATION,
    WIZ_STONE_TO_FLESH,
    WIZ_FLESH_TO_STONE,
    WIZ_TENSERS_TRANSFORMATION,
    WIZ_CONJURE_GREATER_ELEMENTAL,
    WIZ_CONTROL_UNDEAD,
    WIZ_DELAYED_BLAST_FIREBALL,
    WIZ_FINGER_OF_DEATH,
    WIZ_FORCE_CAGE,
    WIZ_MASS_INVISIBILITY,
    WIZ_MONSTER_SUMMONING_V,
    WIZ_MORDENKAINENS_SWORD,
    WIZ_POWER_WORD_STUN,
    WIZ_PRISMATIC_SPRAY,
    WIZ_SPELL_TURNING,
    WIZ_BIGBYS_CLENCHED_FIST,
    WIZ_INCENDIARY_CLOUD,
    WIZ_MASS_CHARM,
    WIZ_MIND_BLANK_WIZARD,
    WIZ_MONSTER_SUMMONING_VI,
    WIZ_OTILUKES_TELEKINETIC_SPHERE,
    WIZ_OTTOS_IRRESISTIBLE_DANCE,
    WIZ_POWER_WORD_BLIND,
    WIZ_PRISMATIC_WALL,
    WIZ_SPELL_IMMUNITY,
    WIZ_BIGBYS_CRUSHING_HAND,
    WIZ_CRYSTAL_BRITTLE,
    WIZ_LEVEL_DRAIN,
    WIZ_METEOR_SWARM,
    WIZ_MONSTER_SUMMONING_VII,
    WIZ_MORDENKAINENS_DISJUNCTION,
    WIZ_POWER_WORD_KILL,
    WIZ_PRISMATIC_SPHERE,
    WIZ_TIME_STOP,
    WIZ_DOME_OF_INVULNERABILITY,
    WIZ_MAGICAL_PLAGUE,
    WIZ_RIFT,
    WIZ_WALL_OF_ASH,
    WIZ_MAX,
};

#define WIZ_MAX (200)

enum {
    CLERIC_BLESS,
    CLERIC_CURSE,
    CLERIC_CURE_LIGHT_WOUNDS,
    CLERIC_CAUSE_LIGHT_WOUNDS,
    CLERIC_ENTANGLE,
    CLERIC_INVISIBILITY_TO_UNDEAD,
    CLERIC_MAGICAL_STONE,
    CLERIC_PROTECTION_FROM_EVIL,
    CLERIC_REMOVE_FEAR,
    CLERIC_CAUSE_FEAR,
    CLERIC_SHILLELAGH,
    CLERIC_AID,
    CLERIC_BARKSKIN,
    CLERIC_CHARM_PERSON_OR_MAMMAL,
    CLERIC_DUST_DEVIL,
    CLERIC_FIND_TRAPS,
    CLERIC_FLAME_BLADE,
    CLERIC_HOLD_PERSON,
    CLERIC_RESIST_FIRE,
    CLERIC_RESIST_COLD,
    CLERIC_SPIRITUAL_HAMMER,
    CLERIC_CONJURE_LESSER_AIR_ELEMENTAL,
    CLERIC_CONJURE_LESSER_FIRE_ELEMENTAL,
    CLERIC_CONJURE_LESSER_EARTH_ELEMENTAL,
    CLERIC_CONJURE_LESSER_WATER_ELEMENTAL,
    CLERIC_CURE_BLINDNESS_OR_DEAFNESS,
    CLERIC_CAUSE_BLINDNESS_OR_DEAFNESS,
    CLERIC_CURE_DISEASE,
    CLERIC_CAUSE_DISEASE,
    CLERIC_DISPEL_MAGIC,
    CLERIC_MAGICAL_VESTMENT,
    CLERIC_NEGATIVE_PLANE_PROTECTION,
    CLERIC_PRAYER,
    CLERIC_PROTECTION_FROM_FIRE,
    CLERIC_REMOVE_CURSE,
    CLERIC_BESTOW_CURSE,
    CLERIC_REMOVE_PARALYSIS,
    CLERIC_SUMMON_INSECTS,
    CLERIC_ABJURE,
    CLERIC_BLOOD_FLOW,
    CLERIC_CLOAK_OF_BRAVERY,
    CLERIC_CLOAK_OF_FEAR,
    CLERIC_CONDENSE,
    CLERIC_CURE_SERIOUS_WOUNDS,
    CLERIC_CAUSE_SERIOUS_WOUNDS,
    CLERIC_DEHYDRATE,
    CLERIC_DUST_CLOUD,
    CLERIC_FOCUS_HEAT,
    CLERIC_FREE_ACTION,
    CLERIC_NEUTRALIZE_POISON,
    CLERIC_POISON,
    CLERIC_PRODUCE_FIRE,
    CLERIC_PROTECTION_FROM_EVIL_10_RADIUS,
    CLERIC_PROTECTION_FROM_LIGHTNING,
    CLERIC_CONJURE_AIR_ELEMENTAL,
    CLERIC_CONJURE_FIRE_ELEMENTAL,
    CLERIC_CONJURE_EARTH_ELEMENTAL,
    CLERIC_CONJURE_WATER_ELEMENTAL,
    CLERIC_CURE_CRITICAL_WOUNDS,
    CLERIC_CAUSE_CRITICAL_WOUNDS,
    CLERIC_DEFLECTION,
    CLERIC_DISPEL_EVIL,
    CLERIC_FLAME_STRIKE,
    CLERIC_INSECT_PLAGUE,
    CLERIC_IRONSKIN,
    CLERIC_QUICKSAND,
    CLERIC_RAISE_DEAD,
    CLERIC_SLAY_LIVING,
    CLERIC_WALL_OF_FIRE,
    CLERIC_MAX,
};

enum {
    PSIONIC_DETONATE,
    PSIONIC_DISINTEGRATE,
    PSIONIC_PROJECT_FORCE,
    PSIONIC_BALLISTIC_ATTACK,
    PSIONIC_CONTROL_BODY,
    PSIONIC_INERTIAL_BARRIER,
    PSIONIC_ANIMAL_AFFINITY,
    PSIONIC_ENERGY_CONTAINMENT,
    PSIONIC_LIFE_DRAINING,
    PSIONIC_ABSORB_DISEASE,
    PSIONIC_ADRENALIN_CONTROL,
    PSIONIC_BIOFEEDBACK,
    PSIONIC_BODY_WEAPONRY,
    PSIONIC_CELL_ADJUSTMENT,
    PSIONIC_DISPLACEMENT,
    PSIONIC_ENHANCED_STRENGTH,
    PSIONIC_FLESH_ARMOR,
    PSIONIC_GRAFT_WEAPON,
    PSIONIC_LEND_HEALTH,
    PSIONIC_SHARE_STRENGTH,
    PSIONIC_DOMINATION,
    PSIONIC_MASS_DOMINATION,
    PSIONIC_PSYCHIC_CRUSH,
    PSIONIC_SUPERIOR_INVISIBILITY,
    PSIONIC_TOWER_OF_IRON_WILL,
    PSIONIC_EGO_WHIP,
    PSIONIC_ID_INSINUATION,
    PSIONIC_INTELLECT_FORTRESS,
    PSIONIC_MENTAL_BARRIER,
    PSIONIC_MIND_BAR,
    PSIONIC_MIND_BLANK,
    PSIONIC_PSIONIC_BLAST,
    PSIONIC_SYNAPTIC_STATIC,
    PSIONIC_THOUGHT_SHIELD,
    PSIONIC_MAX,
};

enum {
    UNKNOWN_RESTORE_PSIONIC_STRENGTH_POINTS,
    UNKNOWN_INCREASE_STRENGTH,
    UNKNOWN_INCREASE_DEXTERITY,
    UNKNOWN_INCREASE_CONSTITUTION,
    UNKNOWN_INCREASE_INTELLIGENCE,
    UNKNOWN_INCREASE_WISDOM,
    UNKNOWN_INCREASE_CHARISMA,
};

typedef struct ssi_spell_list_s {
    uint8_t spells[MAX_SPELLS/8];
} ssi_spell_list_t;

extern void spell_get_psionic_name(uint8_t psi, char name[32]);
extern void spell_get_wizard_name(uint8_t spell, char name[32]);
extern void spell_get_cleric_name(uint8_t spell, char name[32]);
extern void spell_get_psin_name(uint8_t psin, char name[32]);
extern void spell_set_psin(psin_t *psin, const uint8_t psi, const int on);
extern int  spell_has_psin(psin_t *psin, const uint8_t psi);
extern void spell_set_psionic(psionic_list_t *psi, uint16_t power);
extern int  spell_has_psionic(psionic_list_t *psi, uint16_t power);
extern void spell_set_spell(ssi_spell_list_t *psi, uint16_t spell);
extern int  spell_has_spell(ssi_spell_list_t *psi, uint16_t spell);

// NEW INTERFACE
//extern spell_t* spell_get_spell(const uint16_t id);
extern char* spells_read_description(const uint16_t id);

extern void wizard_init();
extern void wizard_add_power(power_t *pw, const int index);
extern void wizard_setup_powers();
extern void wizard_cleanup();
extern power_list_t* wizard_get_spells(const int level);
extern power_t* wizard_get_spell(const int idx);

#endif
