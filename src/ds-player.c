#include <string.h>
#include "gff-common.h"
#include "gfftypes.h"
#include "gff.h"
#include "ds-player.h"
#include "spells.h"

typedef struct player_s {
    ds_character_t ch;
    ds1_combat_t combat;
    psin_t psi;
    spell_list_t spells;
    psionic_list_t psionics;
} player_t;

#define MAX_PCS (4)
#define BUF_MAX (1<<12)

static player_t pc[MAX_PCS];

void ds_player_init() {
    memset(pc, 0x0, MAX_PCS * sizeof(player_t));
}

int ds_player_load_character_charsave(const int slot, const int res_id) {
    char buf[BUF_MAX];
    rdff_header_t *rdff;
    size_t offset = 0;

    if (slot < 0 || slot >= MAX_PCS) { return 0; }

    gff_chunk_header_t chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_CHAR, res_id);
    if (gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &buf, sizeof(buf)) < 34) { return 0; }

    rdff = (rdff_disk_object_t*) (buf);
    memcpy(&(pc[slot].combat), buf + 10, sizeof(ds1_combat_t));
    offset += 10 + rdff->len;

    memcpy(&(pc[slot].ch), buf + 0x4E, sizeof(ds_character_t));

    chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_PSIN, res_id);
    if (!gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &(pc[slot].psi), sizeof(psin_t))) { return 0; }

    chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_SPST, res_id);
    if (!gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &(pc[slot].spells), sizeof(spell_list_t))) { return 0;}

    chunk = gff_find_chunk_header(CHARSAVE_GFF_INDEX, GFF_PSST, res_id);
    if (!gff_read_chunk(CHARSAVE_GFF_INDEX, &chunk, &(pc[slot].psionics), sizeof(psionic_list_t))) { return 0;}

    return 1;
}

int ds_player_exists(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return pc[slot].combat.name[0];
}

ds1_combat_t* ds_player_get_combat(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return &(pc[slot].combat);
}

ds_character_t* ds_player_get_char(const int slot) {
    if (slot < 0 || slot >= MAX_PCS) { return 0; }
    return &(pc[slot].ch);
}
