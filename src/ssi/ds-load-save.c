#include "ds-load-save.h"
#include "player.h"
#include "region.h"
#include "region-manager.h"
#include "gpl-state.h"
#include "gpl.h"
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
    dude_t *dude = sol_player_get(player);
    item_t *item = dude->inv;

    for (int i = 0; i < 26; i++) {
        if (item && item[i].ds_id) {
            num_items++;
        }
    }

    rdff.load_action = RDFF_OBJECT;
    rdff.blocknum = 2 + num_items; // 2 objects (player & name) + items.
    rdff.type = PLAYER_OBJECT;
    rdff.index = id;
    rdff.from = id;
    rdff.len = sizeof(entity_t);
    buf = append(buf, &offset, &buf_len, &rdff, sizeof(rdff_header_t));
    buf = append(buf, &offset, &buf_len, dude, rdff.len);

    if (dude->name) {
        rdff.load_action = RDFF_OBJECT;
        rdff.blocknum = 0;
        rdff.type = ENTITY_NAME;
        rdff.index = id;
        rdff.from = id;
        rdff.len = strlen(dude->name) + 1;
        buf = append(buf, &offset, &buf_len, &rdff, sizeof(rdff_header_t));
        buf = append(buf, &offset, &buf_len, dude->name, rdff.len);
    }

    // Next would be the items: TBD
    for (int i = 0; i < 26; i++) {
        if (item && item[i].ds_id) {
            rdff.load_action = RDFF_OBJECT;
            rdff.blocknum = 0; // no sub objects, TBD: containers?
            rdff.type = FULL_ITEM_OBJECT;
            rdff.index = i;
            rdff.from = item[i].ds_id;
            rdff.len = sizeof(item_t);
            //TODO: Add effects!
            buf = append(buf, &offset, &buf_len, &rdff, sizeof(rdff_header_t));
            buf = append(buf, &offset, &buf_len, item + i, sizeof(item_t));
        }
    }

    rdff.load_action = RDFF_END;
    rdff.blocknum = 0;
    rdff.type = 0;
    rdff.index = 0;
    rdff.from = 0;
    rdff.len = 0;
    buf = append(buf, &offset, &buf_len, &rdff, sizeof(rdff_header_t));

    gff_add_chunk(id, GFF_CHAR, player, buf, offset);
    free(buf);
}

static void save_regions(const int id) {
    sol_region_t *reg = sol_region_manager_get_current();
    dude_t *dude = sol_player_get_active();
    dude_t *entity = NULL;
    size_t buf_len = 128, offset = 0;
    uint32_t len;
    char *buf = malloc(buf_len);
    rdff_header_t rdff;

    if (reg) {
        entity_list_for_each(reg->entities, entity) {
            if (entity->name) {
                rdff.load_action = RDFF_OBJECT;
                rdff.blocknum = 0;
                rdff.type = PLAYER_OBJECT;
                rdff.index = rdff.from = 0;
                rdff.len = sizeof(entity_t);
                buf = append(buf, &offset, &buf_len, &rdff, sizeof(rdff_header_t));
                buf = append(buf, &offset, &buf_len, entity, sizeof(entity_t));
                // Any of his items will need to go here.
            }
        }
        gff_add_chunk(id, GFF_RENT, dude->region, buf, offset);
        offset = 0;
        buf = append(buf, &offset, &buf_len, reg->flags, sizeof(reg->flags));
        //buf = append(buf, &offset, &buf_len, &(reg->cr), sizeof(reg->cr));
        gff_add_chunk(id, GFF_RDAT, dude->region, buf, offset);
    }
    free(buf);

    buf = gpl_serialize_globals(&len);
    gff_add_chunk(id, GFF_GDAT, 99, buf, len);
    free(buf);

    buf = gpl_serialize_locals(&len);
    gff_add_chunk(id, GFF_GDAT, dude->region, buf, len);
    free(buf);
}

//TODO: Will need to save off ALL regions when we get to multiple regions
static void load_regions(const int id) {
    //entity_t *entity = NULL;
    //gff_palette_t *pal = open_files[RESOURCE_GFF_INDEX].pals->palettes;
    dude_t *dude = sol_player_get_active();
    sol_region_t *reg = sol_region_manager_get_region(dude->region);
    char *buf = NULL;
    rdff_header_t *rdff = NULL;
    size_t offset = 0;

    if (reg) {
        gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_RDAT, dude->region);
        buf = malloc(chunk.length);
        if (!buf || (gff_read_chunk(id, &chunk, buf, chunk.length) < chunk.length)) { return; }
        memcpy(&(reg->flags), buf, sizeof(reg->flags));

        chunk = gff_find_chunk_header(id, GFF_RENT, dude->region);
        buf = realloc(buf, chunk.length);
        if (!buf || (gff_read_chunk(id, &chunk, buf, chunk.length) < chunk.length)) { return; }

        offset = 0;
        rdff = (rdff_header_t*)(buf + offset);
        while(offset < chunk.length && rdff->type == PLAYER_OBJECT) {
            offset += sizeof(rdff_header_t);
            entity_t* dude = entity_create_clone((entity_t*)(buf + offset));
            //printf("dude->name = %s, hunt = %d, (%d, %d)\n", dude->name, dude->abilities.hunt,
                //dude->mapx, dude->mapy);

            entity_list_add(reg->entities, dude);
            //port_add_entity(dude, pal);

            offset += sizeof(entity_t);
            rdff = (rdff_header_t*)(buf + offset);
        }
        free(buf);
    }

    port_change_region(reg);
}

extern void ls_save_to_file(const char *path, char *save_name) {
    int id = gff_create(path);

    gff_add_type(id, GFF_PSIN);
    gff_add_type(id, GFF_PSST);
    gff_add_type(id, GFF_SPST);
    gff_add_type(id, GFF_CHAR);
    gff_add_type(id, GFF_POS);
    gff_add_type(id, GFF_STXT);
    gff_add_type(id, GFF_RENT); // Region Entities
    gff_add_type(id, GFF_RDAT); // Region data, not compatible with DS1.
    gff_add_type(id, GFF_TRIG); // Trigger Objects
    gff_add_type(id, GFF_GDAT); // GPL data Objects

    for (int i = 0; i < 4; i++) {
        //gff_add_chunk(id, GFF_PSIN, i, (char*)ds_player_get_psi(i), sizeof(psin_t));
        //gff_add_chunk(id, GFF_PSST, i, (char*)ds_player_get_psionics(i), sizeof(psionic_list_t));
        //gff_add_chunk(id, GFF_SPST, i, (char*)ds_player_get_spells(i), sizeof(ssi_spell_list_t));
        add_player_to_save(id, i);
    }

    save_regions(id);

    size_t trigger_len;
    char* triggers = sol_trigger_serialize(&trigger_len);
    gff_add_chunk(id, GFF_TRIG, 0, triggers, trigger_len);

    gff_add_chunk(id, GFF_STXT, 0, save_name, strlen(save_name) + 1);

    free(triggers);
    gff_close(id);
}

extern char* ls_create_save_file(char *name) {
    char *path = get_next_save_file();
    ls_save_to_file(path, name);
    return path;
}

#define BUF_MAX (1<<12)

static int load_player(const int id, const int player, const int res_id) {
    char buf[BUF_MAX];
    rdff_header_t *rdff;
    size_t offset = 0;
    dude_t *dude = NULL;
    gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_CHAR, res_id);
    if (gff_read_chunk(id, &chunk, &buf, sizeof(buf)) < 34) { return 0; }

    sol_player_free(player);
    dude = sol_player_get(player);
    if (dude) { entity_free (dude); }
    sol_player_set(player, entity_create_fake(30, 10));
    dude = sol_player_get(player);
    entity_load_from_gff(dude, id, player, res_id);

    rdff = (rdff_disk_object_t*) (buf);
    offset += sizeof(rdff_disk_object_t);
    if (rdff->type == COMBAT_OBJECT) {
        warn("Combat object encountered in save. Ignoring.\n");
        offset += rdff->len;

        rdff = (rdff_disk_object_t*) (buf + offset);
        offset += sizeof(rdff_disk_object_t);
        offset += rdff->len;
    } else if (rdff->type == PLAYER_OBJECT) {
        sol_player_free(player);
        dude = sol_player_get(player);
        entity_load_from_object(dude, buf + offset);
        dude->anim.scmd = sol_combat_get_scmd(COMBAT_SCMD_STAND_DOWN);
        offset += rdff->len;
    }

    rdff = (rdff_disk_object_t*) (buf + offset);
    
    if (rdff->type == ENTITY_NAME) {
        offset += sizeof(rdff_disk_object_t);
        dude->name = strdup(buf + offset);
        offset += rdff->len;
    }

    chunk = gff_find_chunk_header(id, GFF_PSIN, res_id);
    //if (!gff_read_chunk(id, &chunk, ds_player_get_psi(player), sizeof(psin_t))) { return 0; }

    chunk = gff_find_chunk_header(id, GFF_SPST, res_id);
    //if (!gff_read_chunk(id, &chunk, ds_player_get_spells(player), sizeof(ssi_spell_list_t))) { return 0;}

    chunk = gff_find_chunk_header(id, GFF_PSST, res_id);
    //if (!gff_read_chunk(id, &chunk, ds_player_get_psionics(player), sizeof(psionic_list_t))) { return 0;}

    return 1;
}

extern int ds_load_character_charsave(const int slot, const int res_id) {
    if (slot < 0 || slot >= 4) { return 0; }
    return load_player(CHARSAVE_GFF_INDEX, slot, res_id);
}

extern int ls_load_save_file(const char *path) {
    int id = gff_open(path);
    char *triggers = NULL;
    char *buf = NULL;


    if (id < 0) { return 0; }

    sol_region_manager_cleanup(0);

    for (int i = 0; i < 4; i++) {
        if (!load_player(id, i, i)) { return 0; }
    }

    load_regions(id);

    gff_chunk_header_t chunk = gff_find_chunk_header(id, GFF_GDAT, 99);
    buf = malloc(chunk.length);
    if (!gff_read_chunk(id, &chunk, buf, chunk.length)) {
        printf("Error loading file.\n");
        exit(1);
    }
    gpl_deserialize_globals(buf);
    free(buf);

    chunk = gff_find_chunk_header(id, GFF_GDAT, sol_player_get_active()->region);
    buf = malloc(chunk.length);
    if (!gff_read_chunk(id, &chunk, buf, chunk.length)) {
        printf("Error loading file.\n");
        exit(1);
    }
    gpl_deserialize_locals(buf);
    free(buf);

    chunk = gff_find_chunk_header(id, GFF_TRIG, 0);
    triggers = malloc(chunk.length);
    if (gff_read_chunk(id, &chunk, triggers, chunk.length) < 1) { return 0; }
    sol_trigger_deserialize(triggers);
    free(triggers);
    triggers = NULL;

    gff_close(id);

    return 1;
}
