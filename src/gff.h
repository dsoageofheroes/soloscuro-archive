#ifndef GFF_H
#define GFF_H

#include <stdint.h>
#include <stdio.h>
#include "gff-image.h"
#include "gff-map.h"

#define GFFTYPE(d,c,b,a) \
  (((long)(a)<<24) + ((long)(b)<<16) + ((long)(c)<<8) + (long)(d))

#define GFFVERSION     (0x00030000L)
#define GFFMAXCHUNKMASK  (0x7FFFFFFFL)
#define GFFSEGFLAGMASK   (0x80000000L)
#define NUM_FILES (256)

enum {
    RESOURCE_GFF_INDEX,
    OBJEX_GFF_INDEX,
    DSLDATA_GFF_INDEX,
    CHARSAVE_GFF_INDEX,
    DARKSAVE_GFF_INDEX,
    CINE_GFF_INDEX,
    DARKRUN_GFF_INDEX,
    REST_GFF_INDEX
};


typedef struct {

  uint32_t	  identity;
  uint32_t	  version;
  uint32_t 	  dataLocation;
  uint32_t 	  tocLocation;
  uint32_t 	  tocLength;
  uint32_t 	  fileFlags;
  uint32_t 	  reserve1;

} gff_file_header_t;

typedef struct {

  int32_t	  chunkId;
  uint32_t 	  chunkDataLocation;
  uint32_t 	  chunkLength;

} gff_chunk_header_t;

#define GFFCHUNKHEADERSIZE ((uint32_t)sizeof(gff_chunk_header_t))

typedef struct {

  uint32_t	  chunkType;
  uint32_t 	  chunkCount;

} gff_chunk_list_header_t;

#define GFFCHUNKLISTHEADERSIZE ((uint32_t)sizeof(gff_chunk_list_header_t))

typedef struct {

  uint32_t	  chunkType;
  uint32_t 	  chunkCount;
  gff_chunk_header_t	  chunks[1];

} gff_chunk_list_t;

typedef struct {

  uint16_t 	  numTypes;
  gff_chunk_list_t   chunks;

} gff_type_header_t;


typedef struct {

  uint32_t 	  typesOffset;	// Byte offset from a pointer to itself.
  uint32_t 	  freeListOffset; // Byte offset from &typesOffset

} gff_toc_header_t;

typedef struct {
  uint32_t	  identity;	// Always 'GFFI'
  int16_t 	  fileId;
  int16_t 	  flags;	// See Masks Above
  gff_file_header_t	  fileHeader;
  uint32_t 	  tocPadLen;	// How much pad?
  gff_toc_header_t	  tocHeader;


} gff_int_file_t;

//#define kGffIntFileSize ((uint32_t)sizeof(gff_int_file_t))

typedef struct {

  uint32_t 	  location;
  uint32_t 	  length;

} gff_free_entry_t;

//#define kGffFreeEntrySize ((uint32_t)sizeof(gff_free_entry_t))

typedef struct {

  uint16_t 	  freeCt;
  gff_free_entry_t   freeChunks[1];

} gff_free_list_t;


typedef struct {

  int32_t	  firstId;
  int32_t 	  consecChunks;

} gff_seg_ref_entry_t;

#define GFFSEGREFENTRYSIZE ((uint32_t)sizeof(gff_seg_ref_entry_t))

typedef struct {

  int32_t 	  numEntries;
  gff_seg_ref_entry_t entries[1];

} gff_seg_ref_struct_t;

typedef struct {

  int32_t 	  segOffset;
  int32_t 	  segLength;

} gff_seg_loc_entry_t;

#define GFFSEGLOCENTRYSIZE ((uint32_t)sizeof(gff_seg_loc_entry_t))

typedef struct {

  int32_t 	  segCount;
  gff_seg_loc_entry_t segEntries[1];

} gff_seg_loc_struct_t;

typedef struct {

  int32_t 	    segCount;
  int32_t	    segLocId;
  gff_seg_ref_struct_t  segRef;

} gff_seg_header_t;

typedef struct _gff_file_t {
    void *data;
    gff_chunk_list_t *gffi_data;
    gff_map_t *map;
    int len;
    char *filename;
    gff_palette_t *palettes;
    unsigned int num_palettes;
    size_t start_palette_index;
    int num_objects;
    gff_map_object_t *entry_table;
} gff_file_t;

extern gff_file_t open_files[NUM_FILES];

extern void gff_init();
extern void gff_load_directory(const char *path);
extern int gff_get_master();
extern int gff_create(const char *pathName);
extern int gff_update(const char *path, int id);
extern int gff_open(const char *pathName);
extern const char** gff_list(size_t *len);
extern void get_gff_type_name(unsigned int gff_type, char *type);
extern int gff_find_index(char *name);
extern int gff_get_number_of_types(int idx);
extern int gff_get_type_id(int idx, int type_index);
extern unsigned int gff_get_gff_type_length(int idx, int type_id);
extern unsigned int* gff_get_id_list(int idx, int type_id);
extern char* gff_get_raw_bytes(int idx, int type_id, int res_id, unsigned long *len);
extern int gff_write_raw_bytes(int idx, int type_id, int res_id, const char *path);
extern void gff_print(int idx, FILE *out);
extern void gff_close (int gff_file);
extern void gff_cleanup();
extern size_t gff_get_palette_id(int idx, int palette_num);

// functions for other modules, NOT "public"
extern gff_chunk_list_t* search_for_chunk_by_name(gff_file_t *file, unsigned long name);

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
    uint16_t char_offset[1];
} ds_font_t;

typedef struct _ds_char_t {
    uint16_t width;
    uint8_t  data[1];
} ds_char_t;

#endif
