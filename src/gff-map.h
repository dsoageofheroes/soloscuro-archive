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

#define COMBAT_NAME_SIZE (18)

typedef struct _ds1_combat_t {
    int16_t hp; // At byte pos 0, confirmed
    int16_t psi; // 2, confirmed
    int16_t char_index; // 4, unconfirmed but looks right.
    int16_t id;  // 6, yes, but is id *-1
    int16_t ready_item_index; // 8, to be cleared.
    int16_t weapon_index; // 10, to be cleared
    int16_t pack_index;   // 12, to be cleared
    uint8_t data_block[8]; // just to shift down 8 bytes.
    uint8_t special_attack; // 22, looks probable.
    uint8_t special_defense; // 23, looks probable.
    int16_t icon; // doesn't look right
    int8_t  ac;   // 26, confirmed
    uint8_t move; // 27, confirmed
    uint8_t status;
    uint8_t allegiance;
    uint8_t data;
    int8_t  thac0; // 31, confirmed
    uint8_t priority;
    uint8_t flags;
    uint8_t stats[6]; // 34, confirmed
    //uint8_t direction;
    // WARNING: This is actually 16, but we do 18 as a buffer.
    char    name[COMBAT_NAME_SIZE]; // 40, confirmed
} ds1_combat_t;

typedef struct _ds1_item_t { // Not confirmed at all...
    int16_t  id; // 0, confirmed (but is negative...)
    uint16_t quantity; // ?
    int16_t  next;  // ?
    uint16_t value; // 6, look correct.
    int16_t  pack_index;
    int16_t  item_index;
    int16_t  icon;
    uint16_t charges;
    uint16_t special;
    uint8_t  priority;
    int8_t   slot;
    uint16_t name_index;
    int8_t   adds;
} ds1_item_t;

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

int gff_load_map_tile_ids(int gff_file, int res_id);
int gff_load_map_flags(int gff_file, int res_id);
int gff_load_map(int gff_file);
int gff_map_is_block(int gff_file, int row, int column);
int gff_map_is_actor(int gff_file, int row, int column);
int gff_map_is_danger(int gff_file, int row, int column);
int32_t get_tile_id(int gff_file, int row, int column);
int gff_map_get_num_objects(int gff_index, int res_id);
unsigned char* gff_map_get_object_bmp(int gff_index, int res_id, int obj_id, int *w, int *h);
void gff_map_get_object_location(int gff_index, int res_id, int obj_id, uint16_t *x, uint16_t *y, uint8_t *z);
so_object_t* gff_object_inspect(int gff_index, int res_id);
disk_object_t* gff_get_object(int object_index);

#endif
