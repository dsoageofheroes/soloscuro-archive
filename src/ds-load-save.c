#include "ds-load-save.h"
#include "ds-player.h"
#include "gff.h"
#include "gfftypes.h"
#include "gff-map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *strdup(const char *s); // not standard.

static char* get_next_save_file() {
    int save_num = 0;
    char buf[128];
    FILE *file = NULL;

    snprintf(buf, 128, "save%02d.sav", save_num);
    file = fopen(buf, "rb");

    while (file) {
        fclose(file);
        snprintf(buf, 128, "save%02d.sav", ++save_num);
        file = fopen(buf, "rb");
    }

    return strdup(buf);
}

static char* append(char *buf, size_t *offset, size_t *len, const void *data, const size_t data_len) {
    while ((*offset + data_len) >= *len) {
        *len *= 1.5;
        buf = realloc(buf, *len);
    }

    memcpy(buf + *offset, data, data_len);
    *offset += data_len;

    return buf;
}

static void add_player_to_save(const int id, const int player) {
    size_t buf_len = 128, offset = 0;
    char *buf = malloc(buf_len);
    rdff_header_t rdff;
    int num_items = 0;

    ds1_item_t *item = (ds1_item_t*) ds_player_get_inv(player);
    for (int i = 0; i < 26; i++) {
        if (item[i].id) {
            num_items++;
        }
    }

    // First the combat object
    rdff.load_action = RDFF_OBJECT;
    rdff.blocknum = 2 + num_items; // 2 objects (combat & char) + items.
    rdff.type = COMBAT_OBJECT;
    rdff.index = id;
    rdff.from = id;
    rdff.len = sizeof(ds1_combat_t);
    buf = append(buf, &offset, &buf_len, &rdff, sizeof(rdff_header_t));
    buf = append(buf, &offset, &buf_len, ds_player_get_combat(player), sizeof(ds1_combat_t));

    // Now the player sheet
    rdff.load_action = RDFF_DATA;
    rdff.blocknum = 0; // no sub objects.
    rdff.type = CHAR_OBJECT;
    rdff.index = id;
    rdff.from = id;
    rdff.len = sizeof(ds_character_t);
    buf = append(buf, &offset, &buf_len, &rdff, sizeof(rdff_header_t));
    buf = append(buf, &offset, &buf_len, ds_player_get_char(player), sizeof(ds_character_t));

    // Next would be the items: TBD
    for (int i = 0; i < 26; i++) {
        if (item[i].id) {
            rdff.load_action = RDFF_OBJECT;
            rdff.blocknum = 0; // no sub objects, TBD: containers?
            rdff.type = ITEM_OBJECT;
            rdff.index = item[i].id;
            rdff.from = item[i].id;
            rdff.len = sizeof(ds1_item_t);
            buf = append(buf, &offset, &buf_len, &rdff, sizeof(rdff_header_t));
            buf = append(buf, &offset, &buf_len, item + i, sizeof(ds1_item_t));
        }
    }

    gff_add_chunk(id, GFF_CHAR, player, buf, offset);
    free(buf);
}

char* ls_create_save_file() {
    char *path = get_next_save_file();
    int id = gff_create(path);

    gff_add_type(id, GFF_PSIN);
    gff_add_type(id, GFF_PSST);
    gff_add_type(id, GFF_SPST);
    gff_add_type(id, GFF_CHAR);
    gff_add_type(id, GFF_POS);

    for (int i = 0; i < 4; i++) {
        gff_add_chunk(id, GFF_PSIN, i, (char*)ds_player_get_psi(i), sizeof(psin_t));
        //psin_t* ds_player_get_psi(i);
        gff_add_chunk(id, GFF_PSST, i, (char*)ds_player_get_psionics(i), sizeof(psionic_list_t));
        //psionic_list_t* ds_player_get_psionics(i);
        gff_add_chunk(id, GFF_SPST, i, (char*)ds_player_get_spells(i), sizeof(spell_list_t));
        //spell_list_t* ds_player_get_spells(i);
        add_player_to_save(id, i);
        //ds1_combat_t *combat = ds_player_get_combat(i);
        //ds_character_t* ds_player_get_char(i);
    }

    gff_add_chunk(id, GFF_POS, 0, (char*)ds_player_get_pos(ds_player_get_active()), sizeof(player_pos_t));

    gff_close(id);
    return path;
}

#define BUF_MAX (1<<12)

static int load_player(const int id, const int player, const int res_id) {
    char buf[BUF_MAX];
    rdff_header_t *rdff;
    size_t offset = 0;
    int num_items;
    ds1_item_t *pc_items = (ds1_item_t*)ds_player_get_inv(player);
    gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_CHAR, res_id);
    if (gff_read_chunk(id, &chunk, &buf, sizeof(buf)) < 34) { return 0; }

    rdff = (rdff_disk_object_t*) (buf);
    num_items = rdff->blocknum - 2;
    offset += sizeof(rdff_disk_object_t);
    memcpy(ds_player_get_combat(player), buf + offset, sizeof(ds1_combat_t));
    offset += rdff->len;

    rdff = (rdff_disk_object_t*) (buf + offset);
    offset += sizeof(rdff_disk_object_t);
    memcpy(ds_player_get_char(player), buf + offset, sizeof(ds_character_t));
    offset += rdff->len;

    for (int i = 0; i < num_items; i++) {
        rdff = (rdff_disk_object_t*) (buf + offset);
        offset += sizeof(rdff_disk_object_t);
        int slot = ((ds1_item_t*)(buf + offset))->slot;
        memcpy(pc_items + slot, buf + offset, sizeof(ds1_item_t));
        offset += rdff->len;
    }

    chunk = gff_find_chunk_header(id, GFF_PSIN, res_id);
    if (!gff_read_chunk(id, &chunk, ds_player_get_psi(player), sizeof(psin_t))) { return 0; }

    chunk = gff_find_chunk_header(id, GFF_SPST, res_id);
    if (!gff_read_chunk(id, &chunk, ds_player_get_spells(player), sizeof(spell_list_t))) { return 0;}

    chunk = gff_find_chunk_header(id, GFF_PSST, res_id);
    if (!gff_read_chunk(id, &chunk, ds_player_get_psionics(player), sizeof(psionic_list_t))) { return 0;}
    
    return 1;
}

int ds_load_character_charsave(const int slot, const int res_id) {
    return load_player(CHARSAVE_GFF_INDEX, slot, res_id);
}

int ls_load_save_file(const char *path) {
    int id = gff_open(path);

    if (id < 0) { return 0; }

    for (int i = 0; i < 4; i++) {
        if (!load_player(id, i, i)) { return 0; }
    }

    gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_POS, 0);
    if (gff_read_chunk(id, &chunk, ds_player_get_pos(0), sizeof(player_pos_t)) < 1) { return 0; }
    if (gff_read_chunk(id, &chunk, ds_player_get_pos(1), sizeof(player_pos_t)) < 1) { return 0; }
    if (gff_read_chunk(id, &chunk, ds_player_get_pos(2), sizeof(player_pos_t)) < 1) { return 0; }
    if (gff_read_chunk(id, &chunk, ds_player_get_pos(3), sizeof(player_pos_t)) < 1) { return 0; }

    return 1;
}
