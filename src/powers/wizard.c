#include <string.h>
#include "wizard.h"
#include "powers.h"
#include "gpl.h"
#include "gff.h"
#include "gfftypes.h"

static sol_power_list_t* wizard_spells[10] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static sol_power_t*      wizard_spell_list[WIZ_MAX];

extern sol_power_list_t* wizard_get_spells(const int level) {
    if (level < 1 || level > 10) { return NULL; }
    return wizard_spells[level - 1];
}

extern sol_power_t* wizard_get_spell(const int idx) {
    if (idx < 0 || idx >= WIZ_MAX) { return NULL; }
    //if (idx < 0 || idx >= WIZ_MAX) { return NULL; }
    //if (!wizard_spell_list) { exit(1); }
    //return (power_t*) 0x1000;
    return wizard_spell_list[idx];
}

extern void wizard_init() {
    memset(wizard_spell_list, 0x0, sizeof(sol_power_t*) * WIZ_MAX);
}

extern void wizard_add_power(sol_power_t *pw, const int idx) {
    int index = pw->level - 1;
    if (index < 0 || index > 9) { return; }
    if (!wizard_spells[index]) { sol_power_list_create(&wizard_spells[index]); }
    if (!pw->description) {
        sol_power_free(pw);
        return;
    }
    sol_power_list_add(wizard_spells[index], pw);
    wizard_spell_list[idx] = pw;
    debug("Added %s to wizard level %d (%p)\n", pw->name, pw->level, pw);
}

extern void wizard_cleanup() {
    for (int i = 0; i < 10; i++) {
        if (wizard_spells[i]) {
            sol_power_list_free(wizard_spells[i]);
            wizard_spells[i] = NULL;
        }
    }
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

int spell_has_psin(sol_psin_t *psin, const uint8_t psi) {
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

void spell_set_psin(sol_psin_t *psin, const uint8_t psi, const int on) {
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

void spell_set_psionic(sol_psionic_list_t *psi, uint16_t power) {
    if (power < 0 || power >= PSIONIC_MAX) { return; }
    psi->psionics[power] = 1;
}

int spell_has_psionic(sol_psionic_list_t *psi, uint16_t power) {
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

extern void spells_set_icon(sol_power_t *spell, const uint16_t id) {
    if (!spell) { return; }
    /*
    spell->icon.bmp_id = id;
    spell->icon.xoffset = spell->icon.yoffset = spell->icon.flags = 0;
    spell->icon.anim.scmd = NULL;
    spell->icon.data = NULL;
    */
}

extern void spells_set_thrown(sol_power_t *spell, const uint16_t id) {
    if (!spell) { return; }
    /*
    spell->thrown.bmp_id = id;
    spell->thrown.xoffset = spell->icon.yoffset = spell->icon.flags = 0;
    spell->thrown.anim.scmd = NULL;
    spell->thrown.data = NULL;
    */
}

extern void spells_set_hit(sol_power_t *spell, const uint16_t id) {
    if (!spell) { return; }
    /*
    spell->hit.bmp_id = id;
    spell->hit.xoffset = spell->icon.yoffset = spell->icon.flags = 0;
    spell->hit.anim.scmd = NULL;
    spell->hit.data = NULL;
    */
}

extern char* spells_read_description(const uint16_t id) {
    char *description = NULL;

    gff_chunk_header_t chunk = gff_find_chunk_header(RESOURCE_GFF_INDEX, GFF_SPIN, id);
    if (chunk.length <= 0) { return NULL; }
    description = calloc(1, chunk.length + 1);
    gff_read_chunk(RESOURCE_GFF_INDEX, &chunk, description, chunk.length);

    return description;
}

