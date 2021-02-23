#include "ds-load-save.h"
#include "ds-player.h"
#include "ds-region.h"
#include "gff.h"
#include "gfftypes.h"
#include "gff-map.h"
#include "port.h"
#include "trigger.h"
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

static void save_region(const int id) {
    dsl_region_t *reg = dsl_region_get_current();
    player_pos_t *player =  ds_player_get_pos(ds_player_get_active());

    if (reg) {
        gff_add_chunk(id, GFF_ROBJ, player->map, (char*)reg->list, sizeof(region_list_t));
    }
}

#define RDFF_BUF_SIZE (1<<11)
static void load_region(const int id) {
    char gff_name[32];
    char buf[RDFF_BUF_SIZE];

    snprintf(gff_name, 32, "rgn%x.gff", id);
    int gff_index = gff_find_index(gff_name);
    if (gff_index < 0 ) { return; }

    dsl_region_t *reg = NULL;
    player_pos_t *player = ds_player_get_pos(ds_player_get_active());

    gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_ROBJ, player->map);
    printf("chunk.length = %d\n", chunk.length);
    reg = dsl_load_region(gff_index);

    if (gff_read_chunk(id, &chunk, reg->list, chunk.length) < chunk.length) {
        printf("ERROR READING!\n");
        return ;
    }

    // Need to get correct pointers for the scmd.
    // TODO: clean this up.
    for (int i = 0; i < MAX_REGION_OBJS; i++) {
        //printf("reg->list->objs[i].entry_id = %d, %d\n", reg->list->objs[i].entry_id, reg->list->objs[i].combat_id);
        region_object_t *robj = reg->list->objs + i;
        if (robj->combat_id == 9999) {
            robj->scmd = gff_map_get_object_scmd(gff_index, id,
                robj->entry_id, 0);
            if (robj->scmd == 0) {
                robj->scmd = ds_scmd_empty();
            }
        } else {
            if (robj->entry_id > 0) {
                robj->scmd = combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
                gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_RDFF, robj->entry_id);
                if (!gff_read_chunk(OBJEX_GFF_INDEX, &chunk, buf, chunk.length)) {
                    printf("ERROR can't read %d\n", robj->entry_id);
                    exit(1);
                }
                robj->combat_id = combat_add(&(reg->cr), robj,
                (ds1_combat_t *) (buf + sizeof(rdff_disk_object_t)));
                port_add_obj(reg->list->objs + i);
            }
        }
        robj->data = NULL;
    }

    port_change_region(reg);
    printf("READ!\n");
}

void ls_save_to_file(const char *path) {
    int id = gff_create(path);

    gff_add_type(id, GFF_PSIN);
    gff_add_type(id, GFF_PSST);
    gff_add_type(id, GFF_SPST);
    gff_add_type(id, GFF_CHAR);
    gff_add_type(id, GFF_POS);
    gff_add_type(id, GFF_ROBJ); // Region objects
    gff_add_type(id, GFF_TRIG); // Trigger Objects

    for (int i = 0; i < 4; i++) {
        gff_add_chunk(id, GFF_PSIN, i, (char*)ds_player_get_psi(i), sizeof(psin_t));
        gff_add_chunk(id, GFF_PSST, i, (char*)ds_player_get_psionics(i), sizeof(psionic_list_t));
        gff_add_chunk(id, GFF_SPST, i, (char*)ds_player_get_spells(i), sizeof(spell_list_t));
        add_player_to_save(id, i);
    }

    gff_add_chunk(id, GFF_POS, 0, (char*)ds_player_get_pos(ds_player_get_active()), sizeof(player_pos_t));

    save_region(id);

    size_t trigger_len;
    char* triggers = trigger_serialize(&trigger_len);
    gff_add_chunk(id, GFF_TRIG, 0, triggers, trigger_len);

    free(triggers);
    gff_close(id);
}

char* ls_create_save_file() {
    char *path = get_next_save_file();
    ls_save_to_file(path);
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
    if (slot < 0 || slot >= 4) { return 0; }
    return load_player(CHARSAVE_GFF_INDEX, slot, res_id);
}

int ls_load_save_file(const char *path) {
    int id = gff_open(path);
    char *triggers = NULL;;

    if (id < 0) { return 0; }

    for (int i = 0; i < 4; i++) {
        if (!load_player(id, i, i)) { return 0; }
    }

    gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_POS, 0);
    if (gff_read_chunk(id, &chunk, ds_player_get_pos(0), sizeof(player_pos_t)) < 1) { return 0; }
    if (gff_read_chunk(id, &chunk, ds_player_get_pos(1), sizeof(player_pos_t)) < 1) { return 0; }
    if (gff_read_chunk(id, &chunk, ds_player_get_pos(2), sizeof(player_pos_t)) < 1) { return 0; }
    if (gff_read_chunk(id, &chunk, ds_player_get_pos(3), sizeof(player_pos_t)) < 1) { return 0; }

    load_region(id);

    chunk = gff_find_chunk_header(id, GFF_TRIG, 0);
    triggers = malloc(chunk.length);
    if (gff_read_chunk(id, &chunk, triggers, chunk.length) < 1) { return 0; }
    trigger_deserialize(triggers);
    free(triggers);
    triggers = NULL;

    return 1;
}
