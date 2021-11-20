#ifndef GFF_H
#define GFF_H

#include <stdint.h>
#include <stdio.h>
#include "gff-common.h"
#include "gff-image.h"
#include "gff-map.h"

#define GFFTYPE(d,c,b,a) \
  (((long)(a)<<24) + ((long)(b)<<16) + ((long)(c)<<8) + (long)(d))

#define GFFVERSION     (0x00030000L)
#define GFFMAXCHUNKMASK  (0x7FFFFFFFL)
#define GFFSEGFLAGMASK   (0x80000000L)
#define NUM_FILES (256)

enum game_type_t {
    DARKSUN_UNKNOWN,
    DARKSUN_1,
    DARKSUN_2,
    DARKSUN_ONLINE,
};

enum {
    RESOURCE_GFF_INDEX,
    OBJEX_GFF_INDEX,
    DSLDATA_GFF_INDEX,
    CHARSAVE_GFF_INDEX,
    DARKSAVE_GFF_INDEX,
    CINE_GFF_INDEX,
    DARKRUN_GFF_INDEX,
    RESFLOP_GFF_INDEX,
    REST_GFF_INDEX
};

typedef struct {
    uint32_t identity;
    uint32_t version;
    uint32_t data_location;
    uint32_t toc_location;
    uint32_t toc_length;
    uint32_t file_flags;
    uint32_t data0;
} gff_file_header_t;

typedef struct {
    uint32_t chunk_type;
    uint32_t chunk_count;
    gff_chunk_header_t chunks[0];
} gff_chunk_list_t;

typedef struct {
    uint32_t types_offset;    // Byte offset from a pointer to itself.
    uint32_t free_list_offset;// Byte offset from &typesOffset
} gff_toc_header_t;

typedef struct {
    int32_t first_id;
    int32_t consec_chunks;
} gff_seg_ref_entry_t;

typedef struct {
    int32_t num_entries;
    gff_seg_ref_entry_t entries[0];
} gff_seg_ref_struct_t;

typedef struct {
    int32_t seg_offset;
    int32_t seg_length;
} gff_seg_loc_entry_t;

typedef struct {
  int32_t seg_count;
  int32_t seg_loc_id;
  gff_seg_ref_struct_t  segRef;
} gff_seg_header_t;

typedef struct seg_header_s {
    int32_t seg_count;
    int32_t seg_loc_id;
    int32_t num_entries;
} seg_header_t;

typedef struct gff_seg_entry_s {
    int32_t first_id;
    int32_t num_chunks;
} gff_seg_entry_t;

typedef struct gff_seg_s {
    seg_header_t header;
    gff_seg_entry_t segs[];
} gff_seg_t;

typedef struct gff_chunk_entry_s {
    uint32_t chunk_type;
    uint32_t chunk_count;
    union {
        gff_chunk_header_t chunks[0];
        gff_seg_t segs;
    };
} gff_chunk_entry_t;

typedef struct _gff_file_t {
    gff_map_t *map;
    char *filename;
    unsigned int num_palettes;
    size_t start_palette_index;
    int num_objects;
    gff_map_object_t *entry_table;

    // New work for loading only the headers.
    FILE *file;
    gff_file_header_t header;
    gff_toc_header_t toc;
    uint16_t num_types;
    gff_chunk_entry_t **chunks;
    gff_chunk_entry_t *gffi;
    gff_palettes_t *pals;
} gff_file_t;

// Need to find location for these:
#define MAX_MONSTERS_PER_REGION (10)

typedef struct _gff_monster_entry_t {
    int16_t id;
    int16_t level;
} gff_monster_entry_t;

typedef struct _gff_monster_region_t {
    int16_t region;
    gff_monster_entry_t monsters[MAX_MONSTERS_PER_REGION];
} gff_monster_region_t;

extern gff_monster_entry_t* gff_load_monster(int region_id, int monster_id);

typedef struct _ds_font_t {
    uint16_t num;
    uint16_t height;
    uint16_t background_color;
    uint16_t flags;
    uint8_t  colors[256];
    uint16_t char_offset[];
} ds_font_t;

typedef struct _ds_char_t {
    uint16_t width;
    uint8_t  data[0];
} ds_char_t;

extern gff_file_t open_files[NUM_FILES];

extern void               gff_init();
extern void               gff_load_directory(const char *path);
extern int                gff_get_master();
extern int                gff_create(const char *pathName);
extern int                gff_open(const char *pathName);
extern const char**       gff_list(size_t *len);
extern void               gff_get_gff_type_name(unsigned int gff_type, char *type);
extern int                gff_find_index(const char *name);
extern int                gff_get_number_of_types(int idx);
extern int                gff_get_type_id(int idx, int type_index);
extern unsigned int       gff_get_resource_length(int idx, int type_id);
extern size_t             gff_get_resource_ids(int idx, int type_id, unsigned int *ids);
extern unsigned int*      gff_get_id_list(int idx, int type_id);
extern size_t             gff_read_raw_bytes(int idx, int type_id, int res_id, void *buf, const size_t len);
extern gff_chunk_header_t gff_find_chunk_header(int idx, int type_id, int res_id);
extern size_t             gff_read_chunk(int idx, gff_chunk_header_t *chunk, void *buf, const size_t len);
extern size_t             gff_read_chunk_piece(int idx, gff_chunk_header_t *chunk, void *read_buf, const size_t len);
extern size_t             gff_write_chunk(const int idx, const gff_chunk_header_t chunk, const char *path);
extern size_t             gff_add_chunk(const int idx, const int type_id, int res_id, char *buf, const size_t len);
extern size_t             gff_add_type(const int idx, const int type_id);
extern int                gff_write_raw_bytes(int idx, int type_id, int res_id, const char *path); // DEPRECATED?
extern void               gff_print(int idx, FILE *out);
extern void               gff_close (int gff_file);
extern void               gff_cleanup();
extern size_t             gff_get_palette_id(int idx, int palette_num);
extern enum game_type_t   gff_get_game_type();

#endif
