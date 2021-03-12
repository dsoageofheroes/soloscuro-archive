#include <string.h>
#include "spells.h"
#include "gff.h"
#include "gfftypes.h"

typedef struct spell_builder_s {
    const char *name;
    const uint16_t icon;
    const int spell_text_id;
} spell_builder_t;

static spell_builder_t wizard_spells[] = {
    { .name = "Armor",                           .icon = 21000, .spell_text_id =  1},
    { .name = "Burning Hands",                   .icon = 21001, .spell_text_id =  2},
    { .name = "Charm",                           .icon = 21002, .spell_text_id =  3},
    { .name = "Chill",                           .icon = 21003, .spell_text_id =  4},
    { .name = "Color Spray",                     .icon = 21004, .spell_text_id =  5},
    { .name = "Enlarge",                         .icon = 21005, .spell_text_id =  6},
    { .name = "Gaze Reflection",                 .icon = 21006, .spell_text_id =  7},
    { .name = "Grease",                          .icon = 21007, .spell_text_id =  8},
    { .name = "Magic Missile",                   .icon = 21008, .spell_text_id =  9},
    { .name = "Shield",                          .icon = 21009, .spell_text_id = 10},
    { .name = "Shocking Grasp",                  .icon = 21010, .spell_text_id = 11},
    { .name = "Wall of Fog",                     .icon = 21011, .spell_text_id = 12},
    { .name = "Blur",                            .icon = 21012, .spell_text_id = 13},
    { .name = "Detect Invisibility",             .icon = 21013, .spell_text_id = 14},
    { .name = "Flame Sphere",                    .icon = 21014, .spell_text_id = 15},
    { .name = "Flog Cloud",                      .icon = 21015, .spell_text_id = 16},
    { .name = "Glitter Dust ",                   .icon = 21016, .spell_text_id = 17},
    { .name = "Invisibility",                    .icon = 21017, .spell_text_id = 18},
    { .name = "Acid Arrow",                      .icon = 21018, .spell_text_id = 19},
    { .name = "Mirror Image",                    .icon = 21019, .spell_text_id = 20},
    { .name = "Protection from Paralysis",       .icon = 21020, .spell_text_id = 21},
    { .name = "Scare",                           .icon = 21021, .spell_text_id = 22},
    { .name = "Stinking Cloud",                  .icon = 21022, .spell_text_id = 23},
    { .name = "Strength",                        .icon = 21023, .spell_text_id = 24},
    { .name = "Web",                             .icon = 21024, .spell_text_id = 25},
    { .name = "Blink",                           .icon = 21025, .spell_text_id = 26},
    { .name = "Dispell",                         .icon = 21026, .spell_text_id = 27},
    { .name = "Fireball",                        .icon = 21027, .spell_text_id = 28},
    { .name = "Flame Arrow",                     .icon = 21028, .spell_text_id = 29},
    { .name = "Haste",                           .icon = 21029, .spell_text_id = 30},
    { .name = "Hold Person",                     .icon = 21030, .spell_text_id = 31},
    { .name = "Hold Undead",                     .icon = 21031, .spell_text_id = 32},
    { .name = "Lighting Bolt",                   .icon = 21032, .spell_text_id = 33},
    { .name = "Melf's Minute Meteor",            .icon = 21033, .spell_text_id = 34},
    { .name = "Minor Malison",                   .icon = 21034, .spell_text_id = 35},
    { .name = "Monster Summoning I",             .icon = 21035, .spell_text_id = 36},
    { .name = "Protection from Normal Missiles", .icon = 21036, .spell_text_id = 37},
    { .name = "Slow",                            .icon = 21037, .spell_text_id = 38},
    { .name = "Spirit Armor",                    .icon = 21038, .spell_text_id = 39},
    { .name = "Vampiric Touch",                  .icon = 21039, .spell_text_id = 40},
    { .name = "Charm Monster",                   .icon = 21040, .spell_text_id = 41},
    { .name = "Confusion",                       .icon = 21041, .spell_text_id = 42},
    { .name = "Evard's Black Tentacles",         .icon = 21042, .spell_text_id = 43},
    { .name = "Fear",                            .icon = 21043, .spell_text_id = 44},
    { .name = "Fire Shield",                     .icon = 21044, .spell_text_id = 45},
    { .name = "Ice Storm",                       .icon = 21045, .spell_text_id = 46},
    { .name = "Improved Invisibility",           .icon = 21046, .spell_text_id = 47},
    { .name = "Minor Globe of Invulerability",   .icon = 21047, .spell_text_id = 48},
    { .name = "Minor Spell Turning",             .icon = 21048, .spell_text_id = 49},
    { .name = "Monster Summoning II",            .icon = 21049, .spell_text_id = 50},
    { .name = "Rainbow Pattern",                 .icon = 21050, .spell_text_id = 51},
    { .name = "Solid Fog",                       .icon = 21051, .spell_text_id = 52},
    { .name = "Stoneskin",                       .icon = 21052, .spell_text_id = 53},
    { .name = "Turn Pebble into Boulder",        .icon = 21053, .spell_text_id = 54},
    { .name = "Wall of Fire",                    .icon = 21054, .spell_text_id = 55},
    { .name = "Wall of Ice",                     .icon = 21055, .spell_text_id = 56},
    { .name = "Chaos",                           .icon = 21056, .spell_text_id = 57},
    { .name = "Cloudkill",                       .icon = 21057, .spell_text_id = 58},
    { .name = "Cone of Cold",                    .icon = 21058, .spell_text_id = 59},
    { .name = "Conjure Elemental",               .icon = 21059, .spell_text_id = 60},
    { .name = "Dismissal",                       .icon = 21060, .spell_text_id = 61},
    { .name = "Dominate",                        .icon = 21061, .spell_text_id = 62},
    { .name = "Freeblemind",                     .icon = 21062, .spell_text_id = 63},
    { .name = "Hold Monster",                    .icon = 21063, .spell_text_id = 64},
    { .name = "Lower Resistance",                .icon = 21064, .spell_text_id = 65},
    { .name = "Monster Summoning III",           .icon = 21065, .spell_text_id = 66},
    { .name = "Summon Shadow",                   .icon = 21066, .spell_text_id = 67},
    { .name = "Wall of Force",                   .icon = 21067, .spell_text_id = 68},
    { .name = "Wall of Stone",                   .icon = 21068, .spell_text_id = 69},
};

extern spell_t* spell_get_spell(const uint16_t id) {
    if (id < 0 || id > sizeof(wizard_spells) / sizeof(spell_t)) { return NULL; }
    //return wizard_spells + id;
    return NULL;
}

static void load_name_from_gff(const uint8_t id, const uint16_t offset, const uint8_t max, char name[32]) {
    char buf[1<<10];
    int pos = 0;
    int gff = gff_get_game_type() == DARKSUN_ONLINE
        ? RESFLOP_GFF_INDEX
        : RESOURCE_GFF_INDEX;

    if (id >= max) {
        name[0] = '\0';
        return;
    }

    gff_chunk_header_t chunk = gff_find_chunk_header(gff, GFF_SPIN, offset + id);
    gff_read_chunk(gff, &chunk, buf, sizeof(buf));

    while (pos < 31 && buf[pos] != ':') {
        name[pos] = buf[pos];
        pos++;
    }

    name[pos] = '\0';
}

void spell_get_psionic_name(uint8_t psi, char name[32]) {
    load_name_from_gff(psi, 139, PSIONIC_MAX, name);
}

void spell_get_wizard_name(uint8_t spell, char name[32]) {
    load_name_from_gff(spell, 0, WIZ_MAX, name);
}

void spell_get_cleric_name(uint8_t spell, char name[32]) {
    load_name_from_gff(spell, 68, CLERIC_MAX, name);
}

void spell_get_psin_name(uint8_t psin, char name[32]) {
    switch(psin) {
        case PSIONIC_PSYCHOKINETIC:
            strcpy(name, "Psyhchokinetic");
            break;
        case PSIONIC_PSYCHOMETABOLISM:
            strcpy(name, "Psychometabolism");
            break;
        case PSIONIC_TELEPATH:
            strcpy(name, "Telepathy");
            break;
        case SPHERE_AIR:
            strcpy(name, "Air");
            break;
        case SPHERE_EARTH:
            strcpy(name, "Earth");
            break;
        case SPHERE_FIRE:
            strcpy(name, "Fire");
            break;
        case SPHERE_WATER:
            strcpy(name, "Water");
            break;
        default:
            strcpy(name, "UNKNOWN");
    }
}

int spell_has_psin(psin_t *psin, const uint8_t psi) {
    switch (psi) {
        case PSIONIC_PSYCHOKINETIC:
            return psin->types[0];
            break;
        case PSIONIC_PSYCHOMETABOLISM:
            return psin->types[2];
            break;
        case PSIONIC_TELEPATH:
            return psin->types[4];
            break;
    }

    return 0;
}

void spell_set_psin(psin_t *psin, const uint8_t psi, const int on) {
    switch (psi) {
        case PSIONIC_PSYCHOKINETIC:
            psin->types[0] = (on) ? 1 : 0;
            break;
        case PSIONIC_PSYCHOMETABOLISM:
            psin->types[2] = (on) ? 1 : 0;
            break;
        case PSIONIC_TELEPATH:
            psin->types[4] = (on) ? 1 : 0;
            break;
    }
}

void spell_set_psionic(psionic_list_t *psi, uint16_t power) {
    if (power < 0 || power >= PSIONIC_MAX) { return; }
    psi->psionics[power] = 1;
}

int spell_has_psionic(psionic_list_t *psi, uint16_t power) {
    if (power < 0 || power >= PSIONIC_MAX) { return 0; }
    return psi->psionics[power];
}

void spell_set_spell(ssi_spell_list_t *spells, uint16_t spell) {
    if (spell < 0 || spell >= WIZ_MAX) { return; }
    spells->spells[spell / 8] |= 1<<(spell % 8);
}

int spell_has_spell(ssi_spell_list_t *spells, uint16_t spell) {
    if (spell < 0 || spell >= WIZ_MAX) { return 0; }
    return (spells->spells[spell / 8] >> (spell % 8)) & 0x01;
}
