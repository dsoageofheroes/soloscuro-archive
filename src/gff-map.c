#include <stdlib.h>
#include <string.h>
#include "gff-map.h"
#include "gff.h"
#include "gfftypes.h"

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

#define OBJECT_PRIORITY_MASK (0x07)
#define OBJECT_EXISTS        (0x08)
#define OBJECT_DONT_DRAW     (0x10)
#define OBJECT_ONE_OBJECT    (0x20) // This object is aliased!
#define OBJECT_REDRAW        (0x40)
#define OBJECT_XMIRROR       (0x80) // Need to flip x axis.

typedef struct _disk_object_t {
    uint16_t flags;
    int16_t  xoffset;
    int16_t  yoffset;
    uint16_t xpos;
    uint16_t ypos;
    int8_t   zpos;
    uint8_t  object_index;
    uint16_t bmp_id;
    uint16_t script_id;
} disk_object_t;

static disk_object_t* gff_get_object(int object_index) {
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
        /*
        for (i = 0; i < num_entries; i++) {
            disk_object_t *object = gff_get_object(entry_table[i].index);
            printf("[%d]: object = %p, bmp_id = %u\n", i, object, object->bmp_id);
        }
        */
        open_files[gff_index].num_objects = num_entries;
        return num_entries;
    }
    return 0;
}

unsigned char* gff_map_get_object_bmp(int gff_index, int res_id, int obj_id, int *w, int *h) {
    int num_objects = gff_map_get_num_objects(gff_index, res_id);
    //unsigned long len;
    if (gff_index < 0 || gff_index >= NUM_FILES || obj_id < 0 || obj_id >= num_objects) {
        return NULL;
    }

    gff_map_object_t *entry_table = open_files[gff_index].entry_table;
    if (entry_table == NULL) { return NULL; }
    disk_object_t *disk_object = gff_get_object(entry_table[obj_id].index);

    *w = get_frame_width(OBJEX_GFF_INDEX, GT_BMP, disk_object->bmp_id, 0);
    *h = get_frame_height(OBJEX_GFF_INDEX, GT_BMP, disk_object->bmp_id, 0);
    return get_frame_rgba_with_palette(OBJEX_GFF_INDEX, GT_BMP, disk_object->bmp_id, 0, -1);
}
