#include <stdlib.h>
#include <string.h>
#include "gff-map.h"
#include "gff.h"
#include "gfftypes.h"
#include "dsl.h"

static gff_map_t* get_map(int gff_file) {
    if (gff_file < 0 || gff_file >= NUM_FILES) { return NULL; }

    if (!open_files[gff_file].map) {
        open_files[gff_file].map = malloc(sizeof(gff_map_t));
        if (open_files[gff_file].map) {
            memset(open_files[gff_file].map, 0, sizeof(gff_map_t));
        }
    }

    return open_files[gff_file].map;
}

int gff_load_map_tile_ids(int gff_file, int res_id) {
    unsigned long len;
    gff_map_t *map = get_map(gff_file);
    char* data = gff_get_raw_bytes(gff_file, GT_RMAP, res_id, &len);

    if (!data || len < 1 || !map) { return 0; }

    map->tile_ids_size = len;
    memcpy(map->tile_ids, data, len);

    return 1;
}

int gff_load_map_flags(int gff_file, int res_id) {
    unsigned long len;
    gff_map_t *map = get_map(gff_file);
    char* data = gff_get_raw_bytes(gff_file, GT_GMAP, res_id, &len);

    if (!data || len < 1 || !map) { return 0; }

    map->flags_size = len;
    memcpy(map->flags, data, map->flags_size);

    return 1;
}

int gff_load_map(int gff_file) {
    unsigned int *gmap_ids = gff_get_id_list(gff_file, GT_GMAP);
    unsigned int *rmap_ids = gff_get_id_list(gff_file, GT_RMAP);
    if (gmap_ids && rmap_ids) {
        gff_load_map_flags(gff_file, gmap_ids[0]);
        gff_load_map_tile_ids(gff_file, rmap_ids[0]);
    }

    if (gmap_ids) { free(gmap_ids); }
    if (rmap_ids) { free(rmap_ids); }

    return 0;
}

int32_t get_tile_id(int gff_file, int row, int column) {
    gff_map_t *map = get_map(gff_file);
    int pos = row*MAP_COLUMNS + column;

    if (!map || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= map->tile_ids_size) {
        return -1;
    }

    return (int32_t)(map->tile_ids[row][column]);
}

int gff_map_is_block(int gff_file, int row, int column) {
    gff_map_t *map = get_map(gff_file);
    int pos = row*MAP_COLUMNS + column;

    if (!map || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= map->tile_ids_size) {
        return -1;
    }

    return (int32_t)(map->flags[row][column] & MAP_BLOCK);
}

int gff_map_is_actor(int gff_file, int row, int column) {
    gff_map_t *map = get_map(gff_file);
    int pos = row*MAP_COLUMNS + column;

    if (!map || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= map->tile_ids_size) {
        return -1;
    }

    return (int32_t)(map->flags[row][column] & MAP_ACTOR);
}

int gff_map_is_danger(int gff_file, int row, int column) { gff_map_t *map = get_map(gff_file);
    int pos = row*MAP_COLUMNS + column;

    if (!map || !VALID_MAP_ROW(row) || !VALID_MAP_COLUMN(column) 
        || pos >= map->tile_ids_size) {
        return -1;
    }

    return (int32_t)(map->flags[row][column] & MAP_DANGER);
}

disk_object_t* gff_get_object(int object_index) {
    unsigned long len;
    if (!open_files[OBJEX_GFF_INDEX].data) {
        return NULL;
    }

    if (object_index < 0) { object_index *= -1; }

    return (disk_object_t*)gff_get_raw_bytes(OBJEX_GFF_INDEX, GT_OJFF, object_index, &len);
}

int gff_map_get_num_objects(int gff_index, int res_id) {
    if (open_files[gff_index].num_objects >= 0) {
        return open_files[gff_index].num_objects;
    }

    unsigned long len;
    unsigned int num_entries, i, j;
    gff_map_object_t *entry_table = (gff_map_object_t*) gff_get_raw_bytes(gff_index, GT_ETAB, res_id, &len);
    gff_map_object_t *centry = entry_table;

    if (entry_table) {
        open_files[gff_index].entry_table = entry_table;
        for (num_entries = 0; centry->index && num_entries < MAX_MAP_OBJECTS; centry++, num_entries++) {
            // centry->index = (centry->index > 0) ? -centry->index : centry->index; //TODO: Only needed if using disk?
            centry->flags |= OBJECT_EXISTS;
        }
        // TODO: Forwarding Bug Fix.  This should only be need once and then saved later.
        for (i = 0; i < num_entries; i++) {
            if (entry_table[i].flags & OBJECT_ONE_OBJECT) {
                for (j = 0; j < num_entries; j++) {
                    if (entry_table[j].index == entry_table[i].index) {
                        entry_table[j].flags |= OBJECT_ONE_OBJECT;
                    }
                }
            }
        }
        open_files[gff_index].num_objects = num_entries;
        return num_entries;
    }
    return 0;
}

int gff_map_get_object_frame_count(int gff_index, int res_id, int obj_id) {
    int num_objects = gff_map_get_num_objects(gff_index, res_id);
    if (gff_index < 0 || gff_index >= NUM_FILES || obj_id < 0 || obj_id >= num_objects) {
        return -1;
    }

    gff_map_object_t *entry_table = open_files[gff_index].entry_table;
    if (entry_table == NULL) { return -1; }
    disk_object_t *disk_object = gff_get_object(entry_table[obj_id].index);

    return get_frame_count(OBJEX_GFF_INDEX, GT_BMP, disk_object->bmp_id);
}

scmd_t* gff_map_get_object_scmd(int gff_index, int res_id, int obj_id, int scmd_index) {
    int num_objects = gff_map_get_num_objects(gff_index, res_id);
    if (gff_index < 0 || gff_index >= NUM_FILES || obj_id < 0 || obj_id >= num_objects) {
        return NULL;
    }

    gff_map_object_t *entry_table = open_files[gff_index].entry_table;
    if (entry_table == NULL) { return NULL; }
    disk_object_t *disk_object = gff_get_object(entry_table[obj_id].index);
    return dsl_scmd_get(OBJEX_GFF_INDEX, disk_object->script_id, scmd_index);
}

unsigned char* gff_map_get_object_bmp(int gff_index, int res_id, int obj_id, int *w, int *h, int frame_id) {
    int num_objects = gff_map_get_num_objects(gff_index, res_id);
    if (gff_index < 0 || gff_index >= NUM_FILES || obj_id < 0 || obj_id >= num_objects) {
        return NULL;
    }

    gff_map_object_t *entry_table = open_files[gff_index].entry_table;
    if (entry_table == NULL) { return NULL; }
    disk_object_t *disk_object = gff_get_object(entry_table[obj_id].index);

    int num_frames = get_frame_count(OBJEX_GFF_INDEX, GT_BMP, disk_object->bmp_id);
    //printf("num_frames = %d\n", num_frames);
    //printf("frame_id = %d\n", frame_id);
    if (frame_id >= num_frames) {
        printf("ERROR: requesting a frame that out of range!\n");
        return NULL;
    }
    *w = get_frame_width(OBJEX_GFF_INDEX, GT_BMP, disk_object->bmp_id, frame_id);
    *h = get_frame_height(OBJEX_GFF_INDEX, GT_BMP, disk_object->bmp_id, frame_id);
    return get_frame_rgba_with_palette(OBJEX_GFF_INDEX, GT_BMP, disk_object->bmp_id, frame_id, -1);
}

void gff_map_get_object_location(int gff_index, int res_id, int obj_id, uint16_t *x, uint16_t *y, uint8_t *z) {
    int num_objects = gff_map_get_num_objects(gff_index, res_id);
    if (gff_index < 0 || gff_index >= NUM_FILES || obj_id < 0 || obj_id >= num_objects) {
        *x = *y = 0xFFFF;
        return;
    }

    gff_map_object_t *entry = open_files[gff_index].entry_table;
    if (entry == NULL) { *x = *y = 0xFFFF; return; }
    //disk_object_t *disk_object = gff_get_object(entry[obj_id].index);
    //if (disk_object->script_id > 0) {
        //dsl_scmd_print(OBJEX_GFF_INDEX, disk_object->script_id);
    //}
    entry += obj_id;
    *x = entry->xpos;
    *y = entry->ypos;
    *z = entry->zpos;
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
        case MINI_OBJECT:
            printf("MINI\n");
            break;
        default:
            printf("-----------UNKNOWN ITEM TYPE: %d\n", entry->type);
            break;
    }
    return obj;
}

so_object_t* gff_object_inspect(int gff_index, int res_id) {
    unsigned long len;
    so_object_t *ret = NULL;
    char *data = gff_get_raw_bytes(gff_index, GT_RDFF, res_id, &len);
    rdff_disk_object_t *entry = (rdff_disk_object_t*) data;
    //printf ("HERE!!!!%d %d, len = %lu, total = %lu\n", gff_index, res_id, len, len / sizeof(rdff_disk_object_t));
    //printf ("proc = %d, bocknum = %d, type = %d, index = %d, from = %d, len = %d\n",
        //entry->load_action, entry->blocknum, entry->type, entry->index, entry->from, entry->len);
    switch(entry->load_action) {
        case RDFF_OBJECT:
            //printf("ITEM\n");
            data += sizeof(rdff_disk_object_t);
            ret = gff_create_object((char*) data, entry, -1);
            break;
        case RDFF_CONTAINER:
            printf("CONTAINER\n");
            break;
        case RDFF_POINTER:
            printf("POINTER\n");
            break;
        case RDFF_NEXT:
            printf("NEXT\n");
            break;
        case RDFF_END:
            printf("END\n");
            break;
    }
    return ret;
}
