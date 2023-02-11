#include <stdlib.h>
#include <string.h>
#include "gff-map.h"
#include "gff.h"
#include "gfftypes.h"
#include "gpl.h"
#include "region.h"
#include "combat.h"

int gff_read_object(int object_index, disk_object_t *disk_obj) {

    if (!disk_obj) {
        return 0;
    }

    if (object_index < 0) { object_index *= -1; }

    gff_chunk_header_t chunk = gff_find_chunk_header(OBJEX_GFF_INDEX, GFF_OJFF, object_index);
    return gff_read_chunk(OBJEX_GFF_INDEX, &chunk, disk_obj, sizeof(disk_object_t));
}

int gff_map_get_num_objects(int gff_idx, int res_id) {
    if (open_files[gff_idx].num_objects >= 0) {
        return open_files[gff_idx].num_objects;
    }

    if (!open_files[gff_idx].entry_table) {
        gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_ETAB, res_id);
        open_files[gff_idx].entry_table = malloc(chunk.length);
        if (!open_files[gff_idx].entry_table) {
            error ("unable to malloc for entry table!\n");
            exit(1);
        }
        gff_read_chunk(gff_idx, &chunk, open_files[gff_idx].entry_table, chunk.length);
        open_files[gff_idx].num_objects = chunk.length / sizeof(gff_map_object_t);
    }

    return open_files[gff_idx].num_objects;
}

int gff_map_get_object_frame_count(int gff_index, int res_id, int obj_id) {
    int num_objects = gff_map_get_num_objects(gff_index, res_id);
    disk_object_t disk_object;

    if (gff_index < 0 || gff_index >= NUM_FILES || obj_id < 0 || obj_id >= num_objects) {
        return -1;
    }

    gff_map_object_t *entry_table = open_files[gff_index].entry_table;
    if (entry_table == NULL) { return -1; }
    gff_read_object(entry_table[obj_id].index, &disk_object);

    return gff_get_frame_count(OBJEX_GFF_INDEX, GFF_BMP, disk_object.bmp_id);
}

int gff_map_fill_scmd_info(dude_t *dude, int gff_index, int res_id, int obj_id, int scmd_index) {
    int num_objects = gff_map_get_num_objects(gff_index, res_id);
    disk_object_t disk_object;

    if (!dude || gff_index < 0 || gff_index >= NUM_FILES || obj_id < 0 || obj_id >= num_objects) {
        return 0;
    }

    gff_map_object_t *entry_table = open_files[gff_index].entry_table;
    if (entry_table == NULL) { return 0; }
    gff_read_object(entry_table[obj_id].index, &disk_object);

    dude->anim.scmd_info.gff_idx = OBJEX_GFF_INDEX;
    dude->anim.scmd_info.res_id = disk_object.script_id;
    dude->anim.scmd_info.index = scmd_index;

    return 1;
}

extern int gff_map_load_scmd(sol_entity_t *dude) {
    if (!dude) { return 0; }

    //printf("scmd loading: %d, %d, %d\n", dude->anim.scmd_info.gff_idx, dude->anim.scmd_info.res_id, dude->anim.scmd_info.index);
    return (dude->anim.scmd = ssi_scmd_get(dude->anim.scmd_info.gff_idx,
            dude->anim.scmd_info.res_id, dude->anim.scmd_info.index)) != NULL;
}

unsigned char* gff_map_get_object_bmp_pal(int gff_index, int res_id, int obj_id, int *w, int *h, int frame_id,
        int palette_id) {
    int num_objects = gff_map_get_num_objects(gff_index, res_id);
    disk_object_t disk_object;

    if (gff_index < 0 || gff_index >= NUM_FILES || obj_id < 0 || obj_id >= num_objects) {
        return NULL;
    }

    gff_map_object_t *entry_table = open_files[gff_index].entry_table;
    if (entry_table == NULL) { return NULL; }
    gff_read_object(entry_table[obj_id].index, &disk_object);

    int num_frames = gff_get_frame_count(OBJEX_GFF_INDEX, GFF_BMP, disk_object.bmp_id);
    //printf("num_frames = %d\n", num_frames);
    //printf("frame_id = %d\n", frame_id);
    if (frame_id >= num_frames) {
        printf("ERROR: requesting a frame that out of range!\n");
        return NULL;
    }
    *w = gff_get_frame_width(OBJEX_GFF_INDEX, GFF_BMP, disk_object.bmp_id, frame_id);
    *h = gff_get_frame_height(OBJEX_GFF_INDEX, GFF_BMP, disk_object.bmp_id, frame_id);
    return gff_get_frame_rgba_with_palette(OBJEX_GFF_INDEX, GFF_BMP, disk_object.bmp_id, frame_id, palette_id);
}

unsigned char* gff_map_get_object_bmp(int gff_index, int res_id, int obj_id, int *w, int *h, int frame_id) {
    return gff_map_get_object_bmp_pal(gff_index, res_id, obj_id, w, h, frame_id, -1);
}

uint16_t gff_map_get_object_location(int gff_index, int res_id, int obj_id, uint16_t *x, uint16_t *y, uint8_t *z) {
    int num_objects = gff_map_get_num_objects(gff_index, res_id);
    disk_object_t disk_object;

    if (gff_index < 0 || gff_index >= NUM_FILES || obj_id < 0 || obj_id >= num_objects) {
        *x = *y = 0xFFFF;
        return 0;
    }

    gff_map_object_t *entry = open_files[gff_index].entry_table;
    if (entry == NULL) { *x = *y = 0xFFFF; return 0; }
    gff_read_object(entry[obj_id].index, &disk_object);
    //if (disk_object->script_id > 0) {
        //dsl_scmd_print(OBJEX_GFF_INDEX, disk_object->script_id);
    //}
    entry += obj_id;
    *x = entry->xpos;
    *y = entry->ypos;
    *x = entry->xpos - disk_object.xoffset;
    *y = entry->ypos - disk_object.yoffset;
    *z = entry->zpos;

    return disk_object.flags;
}

so_object_t* gff_create_object(char *data, rdff_disk_object_t *entry, int16_t id) {
    int i;
    so_object_t* obj = calloc(1, sizeof(so_object_t)); // calloc zeroizes.
    if (!obj) { return NULL; } // Calloc failed.
    //int16_t *objectHeader;
    //data += 9;
    obj->type = SO_EMPTY;
    switch(entry->type) {
        case COMBAT_OBJECT:
            obj->type = SO_DS1_COMBAT;
            memcpy(&(obj->data.ds1_combat), (ds1_combat_t*) data, sizeof(ds1_combat_t) - 16); // Don't copy the name over!
            ds1_combat_t *combat = &(obj->data.ds1_combat);
            for (i = 0; i < 17 && ((ds1_combat_t*)data)->name[i]; i++) {
                combat->name[i] = ((ds1_combat_t*)data)->name[i];
            }
            // Force the name to be null-terminated.
            i = i >= 17 ? 16 : i;
            combat->name[i] = '\0';
            //printf("name = %s\n", combat->name);
            /*
            printf("\n[%d]:", 0);
            for (int i = 0; i < sizeof(ds1_combat_t); i++) {
                printf("%d, ", ((char*)combat)[i]);
                if ((i % 16) == 15) { printf("\n[%d]:", i+1); }
            }
            printf("\n");
            printf("ac = %d, movement = %d, thac0 = %d, hp = %d, psi = %d"
                "special attack = %d\n", 
                combat->ac, combat->move, combat->thac0, combat->hp, combat->psi,
                combat->special_attack);
            */
            combat->char_index = NULL_OBJECT;
            combat->ready_item_index = NULL_OBJECT;
            combat->weapon_index = NULL_OBJECT;
            combat->pack_index = NULL_OBJECT;
            // Always start at least with an OK combat status.
            if (combat->status == 0) {
                combat->status = COMBAT_STATUS_OK;
            }
            break;
        case ITEM_OBJECT:
            obj->type = SO_DS1_ITEM;
            ds1_item_t *item = &(obj->data.ds1_item);
            memcpy(item, (ds1_item_t*) data, sizeof(ds1_item_t));
            /*
            printf("id = %d\n", item->id);
            printf("quantity = %d\n", item->quantity);
            printf("next = %d\n", item->next);
            printf("value = %d\n", item->value);
            printf("pack_index = %d\n", item->item_index);
            printf("icon = %d\n", item->icon);
            printf("charges = %d\n", item->charges);
            printf("special = %d\n", item->special);
            printf("slot = %d\n", item->slot);
            printf("name_index = %d\n", item->name_index);
            printf("adds = %d\n", item->adds);
            disk_object_t* diskobject = gff_get_object(item->id);
            printf("bmpid = %d\n", diskobject->bmp_id);
            printf("script_id = %d\n", diskobject->script_id);
            */
            break;
        case CHAR_OBJECT:
            printf("CHARACTER!!!!!!!!!!!!!!!!!!!!\n");
            exit(1);
            break;
        case MINI_OBJECT:
            printf("MINI\n");
            break;
        default:
            printf("-----------UNKNOWN ITEM TYPE: %d\n", entry->type);
            break;
    }

    return obj;
}

#define RDFF_MAX (1<<12)
so_object_t* gff_object_inspect(int gff_idx, int res_id) {
    so_object_t *ret = NULL;
    gff_idx = OBJEX_GFF_INDEX;
    char *tmp = NULL;
    rdff_disk_object_t rdff_buf[RDFF_MAX];

    rdff_disk_object_t *rdff = rdff_buf;
    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_RDFF, res_id);

    if (chunk.length > RDFF_MAX * sizeof(rdff_disk_object_t)) {
        error ("chunk.length (%d) greater than max (%d)!\n", chunk.length, RDFF_MAX);
        exit(1);
    }

    gff_read_chunk(gff_idx, &chunk, rdff, chunk.length);
    rdff_disk_object_t *entry = rdff;
    char *data = (char*)entry;

    while (entry->load_action != -1) {
        //printf ("proc = %d, bocknum = %d, type = %d, index = %d, from = %d, len = %d\n",
                //entry->load_action, entry->blocknum, entry->type, entry->index, entry->from, entry->len);
        switch(entry->load_action) {
            case RDFF_OBJECT:
                //printf("OBJECT\n");
                data += sizeof(rdff_disk_object_t);
                ret = gff_create_object((char*) data, entry, -1);
                return ret;
                break;
            case RDFF_CONTAINER:
                printf("CONTAINER\n");
                break;
            case RDFF_DATA:
                //printf("DATA\n");
                break;
            case RDFF_NEXT:
                printf("NEXT\n");
                break;
            case RDFF_END:
                printf("END\n");
                break;
        }
    
        // Iterate to the next object
        tmp = (char*)entry;
        tmp += sizeof(rdff_disk_object_t);
        tmp += entry->len;
        entry = (rdff_disk_object_t *)tmp;
    }

    return ret;
}
