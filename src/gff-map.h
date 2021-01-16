#ifndef GFF_MAP_H
#define GFF_MAP_H

#include <stdint.h>

#include "dsl-scmd.h"
#include "dsl-object.h"

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
#define OBJECT_PRIORITY_MASK (0x07)
#define OBJECT_EXISTS        (0x08)
#define OBJECT_DONT_DRAW     (0x10)
#define OBJECT_ONE_OBJECT    (0x20) // This object is aliased!
#define OBJECT_REDRAW        (0x40)
#define OBJECT_XMIRROR       (0x80) // Need to flip x axis.

typedef struct _disk_object_s {
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
    int16_t  index; // bit 15: item is on disk(segobjx.gff), abs(index) = chunk id!
} gff_map_object_t;

// The small 'n' means next. IE: nbmp_idx = next bitmap index.
typedef struct _gff_script_cmd_t {
    uint8_t nbmp_idx;     // 0-254 = valid, 255 invalid.
    uint8_t nbmp_delay;   // 0-255 = valid
    uint8_t flags;
    int8_t  nxoffset;
    int8_t  nyoffset;
    int8_t  npxoffset;
    int8_t  npyoffset;
    uint8_t sound_index;
} gff_script_cmd_t;

#define MAX_SCRIPTS (240)
#define MAX_SCRIPT_JUMPS (36)
typedef struct _gff_script_jump_t {
    uint16_t jump_ids[MAX_SCRIPTS][MAX_SCRIPT_JUMPS];
} gff_script_jump_t;

typedef struct _gff_script_entry_t {
    uint8_t use_count;
    uint16_t chunk_id;
    gff_script_cmd_t *cmds;
    int16_t size;
} gff_script_entry_t;

extern gff_script_entry_t script_entry_table[MAX_SCRIPTS];

typedef struct _rdff_disk_object_t {
    int8_t  load_action;
    int8_t  blocknum;
    int16_t type;
    int16_t index;
    int16_t from;
    int16_t len;
} rdff_disk_object_t;

enum {
    RDFF_OBJECT    = 1,
    RDFF_CONTAINER = 2,
    RDFF_POINTER   = 3,
    RDFF_NEXT      = 4,
    RDFF_END       =-1
};

enum {
    ITEM_OBJECT   = 1,
    COMBAT_OBJECT = 2,
    MINI_OBJECT   = 5,
};

// Types of Sol Oscuro objects
enum {
    SO_EMPTY = 0, // Empty: free to overide!
    SO_DS1_COMBAT,
    SO_DS1_ITEM
};

typedef struct _so_object_t {
    uint16_t type;
    union {
        ds1_combat_t ds1_combat;
        ds1_item_t   ds1_item;
    } data;
} so_object_t;

#define NULL_OBJECT (9999) // why 9999?  I dunno!

enum {
    COMBAT_STATUS_OK          = 1,
    COMBAT_STATUS_STUNNED     = 2,
    COMBAT_STATUS_UNCONSCIOUS = 3,
    COMBAT_STATUS_DYING       = 4,
    COMBAT_STATUS_ANIMATED    = 5,
    COMBAT_STATUS_PETRIFIED   = 6,
    COMBAT_STATUS_DEAD        = 7,
    COMBAT_STATUS_GONE        = 8,
    COMBAT_STATUS_MAX
};

//int gff_load_map_tile_ids(int gff_file, int res_id);
//int gff_load_map_flags(int gff_file, int res_id);
//int gff_load_map(int gff_file);
//int32_t get_tile_id(int gff_file, int row, int column);
int gff_map_get_num_objects(int gff_index, int res_id);
int gff_map_get_object_frame_count(int gff_index, int res_id, int obj_id);
scmd_t* gff_map_get_object_scmd(int gff_index, int res_id, int obj_id, int scmd_index);
unsigned char* gff_map_get_object_bmp(int gff_index, int res_id, int obj_id, int *w, int *h, int frame_id);
unsigned char* gff_map_get_object_bmp_pal(int gff_index, int res_id, int obj_id, int *w, int *h, int frame_id,
        int palette_id);
uint16_t gff_map_get_object_location(int gff_index, int res_id, int obj_id, uint16_t *x, uint16_t *y, uint8_t *z);
so_object_t* gff_object_inspect(int gff_index, int res_id);
so_object_t* gff_create_object(char *data, rdff_disk_object_t *entry, int16_t id);
int gff_read_object(int object_index, disk_object_t *disk_obj);

#endif
