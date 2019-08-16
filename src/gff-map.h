#ifndef GFF_MAP_H
#define GFF_MAP_H

#include <stdint.h>

#define MAP_LOS     (0x80) // Runtime flag to determine if block is in sight.
#define MAP_BLOCK   (0x40) // This tile is blocked (you can't move into it.)
#define MAP_ACTOR   (0x20) // If there is actor here (and is blocked.)
#define MAP_DANGER  (0x07) // If it is dangerous and you are not courageous, you are blocked
#define MAP_GPL     (0x04) // ????

#define MAP_ROWS 98
#define MAP_COLUMNS 128

#define MAX_MAP_OBJECTS (2000)

#define VALID_MAP_ROW(row) (row >= 0 && row < MAP_ROWS)
#define VALID_MAP_COLUMN(col) (col >= 0 && col < MAP_COLUMNS)
#define HAS_MAP_DATA(gff_index) (gff_index >= 0 && gff_file < NUM_FILES && open_files[gff_index].map)

typedef struct _gff_map_t {
    uint8_t flags[MAP_ROWS][MAP_COLUMNS];
    uint8_t tile_ids[MAP_ROWS][MAP_COLUMNS];
    uint32_t flags_size, tile_ids_size;
} gff_map_t;

typedef struct _gff_map_object_t {
    uint16_t xpos;
    uint16_t ypos;
    int8_t   zpos;
    uint8_t  flags;
    int16_t  index;
} gff_map_object_t;

int gff_load_map_tile_ids(int gff_file, int res_id);
int gff_load_map_flags(int gff_file, int res_id);
int gff_load_map(int gff_file);
int gff_map_is_block(int gff_file, int row, int column);
int gff_map_is_actor(int gff_file, int row, int column);
int gff_map_is_danger(int gff_file, int row, int column);
int32_t get_tile_id(int gff_file, int row, int column);
int gff_map_get_num_objects(int gff_index, int res_id);
unsigned char* gff_map_get_object_bmp(int gff_index, int res_id, int obj_id, int *w, int *h);

#endif
