#include "gff.h"
#include "gfftypes.h"
#include "gff-image.h"
#include "dsl.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#define BUF_SIZE (1024)
gff_file_t open_files[NUM_FILES];
static int master_gff;

static int write_toc(const int gff_idx);
static int get_next_idx(const char *name);

int gff_get_master() {
    return master_gff;
}

static char* strtolwr(char *str) {

    for (char *c = str; *c; c++) {
        *(c) = tolower(*c);
    }

    return str;
}

// Precondition, name is in lowercase!
int gff_find_index(const char *name) {
    if (!name) { return -1; }

    for (int i = 0; i < NUM_FILES; i++) {
        //printf("comparing: '%s' ?= '%s'\n", name, open_files[i].filename);
        if (open_files[i].filename 
            && strcmp(name, open_files[i].filename) == 0) {
            return i;
        }
    }

    return -1;
}

// Precondition, name is in lowercase!
int get_next_idx(const char *name) {
    int i;

    if (gff_find_index(name) > -1) {
        fprintf(stderr, "'%s' is already loaded!\n", name);
        return -1; // We already have that file!
    }

    if (strcmp(name, "resource.gff") == 0) { return RESOURCE_GFF_INDEX; }
    if (strcmp(name, "segobjex.gff") == 0) { return OBJEX_GFF_INDEX; }
    if (strcmp(name, "objex.gff") == 0) { return OBJEX_GFF_INDEX; }
    if (strcmp(name, "gpldata.gff") == 0) { return DSLDATA_GFF_INDEX; }
    if (strcmp(name, "charsave.gff") == 0) { return CHARSAVE_GFF_INDEX; }
    if (strcmp(name, "darksave.gff") == 0) { return DARKSAVE_GFF_INDEX; }
    if (strcmp(name, "cine.gff") == 0) { return CINE_GFF_INDEX; }
    if (strcmp(name, "darkrun.gff") == 0) { return DARKRUN_GFF_INDEX; }
    if (strcmp(name, "resflop.gff") == 0) { return RESFLOP_GFF_INDEX; }

    for (i = REST_GFF_INDEX; i < NUM_FILES; i++) {
        if (!open_files[i].file) {
            return i;
        }
    }

    fprintf(stderr, "Number of GFF slots are full!\n");
    return -1;
}

void gff_init() {
    memset(open_files, 0, sizeof(gff_file_t) * NUM_FILES);

    master_gff = -1;
    gff_image_init();
}

static int is_gff_filename(const char *str) {
    int slen = strlen (str);

    return slen > 4 
        && (str[slen-1] == 'f' || str[slen-1] == 'F')
        && (str[slen-2] == 'f' || str[slen-2] == 'F')
        && (str[slen-3] == 'g' || str[slen-3] == 'G');
}

void gff_load_directory(const char *path) {
    DIR *dir;
    struct dirent *ent;
    char buf[BUF_SIZE];

    debug("Loading GFFs from: %s\n", path);
    if ((dir = opendir (path)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if (is_gff_filename(ent->d_name)) {
                snprintf(buf, BUF_SIZE, "%s/%s", path, ent->d_name);
                gff_open(buf);
            }
        }
        closedir (dir);
    } else {
        fprintf(stderr, "Unable to open directory: '%s'\n", path);
    }
}

const enum game_type_t gff_get_game_type() {
    if (open_files[RESOURCE_GFF_INDEX].filename && open_files[RESFLOP_GFF_INDEX].filename) {
        return DARKSUN_2;
    }

    if (open_files[RESOURCE_GFF_INDEX].filename && !open_files[RESFLOP_GFF_INDEX].filename) {
        return DARKSUN_1;
    }

    if (!open_files[RESOURCE_GFF_INDEX].filename && open_files[RESFLOP_GFF_INDEX].filename) {
        return DARKSUN_ONLINE;
    }

    return DARKSUN_UNKNOWN;
}

const char** gff_list(size_t *len) {
    *len = 0;
    const char **ret = NULL;
    int pos = 0;

    for (int i = 0; i < NUM_FILES; i++) {
        if (open_files[i].filename) { (*len)++; }
    }

    if (*len == 0) { return NULL; }

    ret = malloc(sizeof(char*) * *len);
    for (int i = 0; i < NUM_FILES; i++) {
        if (open_files[i].filename) { 
            ret[pos] = open_files[i].filename;
            pos++;
        }
    }

    return ret;
}

int gff_create(const char *path) {
    int id = get_next_idx(path);
    FILE * file = fopen(path, "wb+");
    gff_file_t *gff = open_files + id;

    gff->file = file;
	gff->header.identity        = GFF_GFFI;
	gff->header.version         = GFFVERSION;
	gff->header.data_location   = 28L;
	gff->header.toc_location    = 28L;
	gff->header.toc_length      = 10L; // types is 2B, toc is 8B
	gff->header.file_flags      =  0L;
	gff->header.data0           =  0L;

    gff->toc.types_offset       =  8L;
    gff->toc.free_list_offset   = 10L;

    gff->num_types              = 0L;
    gff->pals = NULL;

    write_toc(id);

    return id;
}

size_t gff_add_type(const int idx, const int type_id) {
    gff_file_t *gff = open_files + idx;
    
    for (int i = 0; i < gff->num_types; i++) {
        if ((gff->chunks[i]->chunk_type & GFFMAXCHUNKMASK) == type_id) {
            return 0; // Type already exists!
        }
    }

    gff->num_types++;
    gff->chunks = realloc(gff->chunks, sizeof(gff_chunk_entry_t*) * gff->num_types);
    gff_chunk_entry_t *new = malloc(8L); // Only allocate the initial 8 bytes worry about extensions later.
    gff->chunks[gff->num_types - 1] = new;
    new->chunk_type = type_id;
    new->chunk_count = 0;

    gff->header.toc_length += 8L;
    gff->toc.free_list_offset += 8L;

    write_toc(idx);

    return 1;
}

static char* get_filename_from_path(const char *path) {
    int len = strlen(path);
    int i = 0;

    if (len < 3) { return NULL; }
    for (i = len-1; i >= 0 && path[i] != '/' && path[i] != '\\' ; i--) {
    }

    return strdup(path + i + 1);
}

static int is_master_name(const char *name) {
    if (strcmp(name, "resource.gff") == 0) { return 1; }
    if (strcmp(name, "resflop.gff") == 0) { return 1; }

    return 0;
}

typedef struct {
  uint32_t	  chunkType;
  uint32_t 	  chunkCount;
} gff_chunk_list_header_t;

static void gff_read_headers(gff_file_t *gff) {
    fseek(gff->file, 0, SEEK_SET);
    if (fread(&(gff->header), 1, sizeof(gff_file_header_t), gff->file)
            != sizeof(gff_file_header_t)) {
        fatal("Unable to read header!\n");
    }

    fseek(gff->file, gff->header.toc_location, SEEK_SET);
    if (fread(&(gff->toc), 1, sizeof(gff_toc_header_t), gff->file)
            != sizeof(gff_toc_header_t)) {
        fatal("Unable to read Table of Contents!\n");
    }

    fseek(gff->file, gff->header.toc_location + gff->toc.types_offset, SEEK_SET);
    //if (fread(&(gff->types), 1, sizeof(gff_type_header_t), gff->file)
            //!= sizeof(gff_type_header_t)) {
    if (fread(&(gff->num_types), 1, sizeof(uint16_t), gff->file)
            != sizeof(uint16_t)) {
        fatal("Unable to read types header!\n");
    }

    gff->chunks = malloc(sizeof(gff_chunk_entry_t*) * gff->num_types);
    memset(gff->chunks, 0x0, sizeof(gff_chunk_entry_t*) * gff->num_types);

    int i = 0;

    gff_chunk_list_header_t chunk_header;
    fseek(gff->file, gff->header.toc_location + gff->toc.types_offset + 2L, SEEK_SET);
    seg_header_t seg_header;
    for (i = 0; i < gff->num_types; i++) {
        fread(&(chunk_header), 1, sizeof(gff_chunk_list_header_t), gff->file);
        gff_chunk_entry_t *chunk = NULL;

        if (chunk_header.chunkCount & GFFSEGFLAGMASK) {
            fread(&(seg_header), 1, sizeof(seg_header_t), gff->file);

            chunk = malloc(8L + sizeof(gff_seg_t) + seg_header.num_entries * sizeof(gff_seg_entry_t));
            chunk->chunk_type = chunk_header.chunkType;
            chunk->chunk_count = chunk_header.chunkCount;
            chunk->segs.header = seg_header;

            fread(&(chunk->segs.segs), 1, seg_header.num_entries * sizeof(gff_seg_entry_t), gff->file);
            gff->chunks[i] = chunk;
        } else {
            chunk = malloc(8L + sizeof(gff_chunk_header_t) * chunk_header.chunkCount);
            chunk->chunk_type = chunk_header.chunkType;
            chunk->chunk_count = chunk_header.chunkCount;
            fread(&(chunk->chunks), 1, sizeof(gff_chunk_header_t) * chunk_header.chunkCount, gff->file);
            gff->chunks[i] = chunk;
        }

        if (chunk && ((chunk->chunk_type & GFFMAXCHUNKMASK) == GFF_GFFI)) {
            gff->gffi = chunk;
        }
    }
}

int gff_open(const char *pathName) {
    int idx;
    FILE *file;
    char *filename = strtolwr(get_filename_from_path(pathName));

    idx = get_next_idx(filename);
    if (idx == -1) { 
        free(filename);
        return idx; 
    }

    file = fopen(pathName, "rb+");
    if (!file) { free(filename); return -1; }
    fseek(file, 0L, SEEK_SET);

    memset(open_files + idx, 0x0, sizeof(gff_file_t));

    //printf("Detected file size of '%s': %d\n", filename, len);
    open_files[idx].num_palettes = 0;
    open_files[idx].num_objects = -1;
    open_files[idx].entry_table = NULL;
    open_files[idx].file = file;

    gff_read_headers(open_files + idx);

    if (is_master_name(filename)) { master_gff = idx; }

    open_files[idx].filename = filename;
    open_files[idx].start_palette_index = gff_get_number_of_palettes();
    gff_create_palettes(idx, &(open_files[idx].num_palettes));
    debug("'%s' loaded as '%s' with id: %d\n", pathName, open_files[idx].filename, idx);

    return idx;
}

int gff_get_number_of_types(int idx) {
    if (open_files[idx].file == NULL) { return -1; }

    return open_files[idx].num_types;
}

int gff_get_type_id(int idx, int type_index) {
    if (open_files[idx].file == NULL || type_index < 0 
            || type_index >= open_files[idx].num_types) { 
        return -1;  // failure.
    }

    return open_files[idx].chunks[type_index]->chunk_type & GFFMAXCHUNKMASK;
}

gff_chunk_header_t gff_find_chunk_header(int idx, int type_id, int res_id) {
    gff_file_t *gff = open_files + idx;
    gff_chunk_entry_t *entry = NULL;
    gff_seg_loc_entry_t seg;
    gff_chunk_header_t *gffi_chunk_header = NULL;
    gff_chunk_header_t ret = {0, 0, 0};

    for (int i = 0; !entry && i < gff->num_types; i++) {
        if (gff->chunks && (gff->chunks[i]->chunk_type & GFFMAXCHUNKMASK) == type_id) {
            entry = gff->chunks[i];
        }
    }
    
    if (!entry) {
        error("Can't find type %d in gff %d\n", type_id, idx);
        return ret;
    }

    if (entry->chunk_count & GFFSEGFLAGMASK) {
        int32_t chunk_offset = 0;
        gffi_chunk_header = gff->gffi->chunks + entry->segs.header.seg_loc_id;
        for (int j = 0; j < entry->segs.header.num_entries; j++) {
            int32_t first_id = entry->segs.segs[j].first_id;
            if (res_id >= first_id && res_id <= (first_id + entry->segs.segs[j].num_chunks)) {
                int offset = 4L + (sizeof(gff_seg_loc_entry_t) * chunk_offset);

                fseek(gff->file, gffi_chunk_header->location + offset + (res_id - first_id)*sizeof(gff_seg_loc_entry_t), SEEK_SET);
                fread(&seg, 1, sizeof(gff_seg_loc_entry_t), gff->file);

                ret.id = res_id;
                ret.location = seg.seg_offset;
                ret.length = seg.seg_length;

                return ret;
            }
            chunk_offset += entry->segs.segs[j].num_chunks;
        }
    } else {
        for (int j = 0; j < entry->chunk_count; j++) {
            if (entry->chunks[j].id == res_id) {
                ret.id = res_id;
                ret.location = entry->chunks[j].location;
                ret.length = entry->chunks[j].length;
                return ret;
            }
        }
    }

    return ret;
}

#define TOC_BUF_SIZE (1<<10)

static int write_toc(const int gff_idx) {
    gff_file_t *gff = open_files + gff_idx;
    static char buf[TOC_BUF_SIZE];

    memset(buf, 0x0, TOC_BUF_SIZE);

    fseek(gff->file, 0, SEEK_SET);
    if (fwrite(&(gff->header), 1, sizeof(gff_file_header_t), gff->file)
            != sizeof(gff_file_header_t)) {
        fatal("Unable to write header!\n");
    }

    fseek(gff->file, gff->header.toc_location, SEEK_SET);
    if ((fwrite(&(gff->toc), 1, sizeof(gff_toc_header_t), gff->file))
            != sizeof(gff_toc_header_t)) {
        fatal("Unable to write Table of Contents header!\n");
    }

    fseek(gff->file, gff->header.toc_location + gff->toc.types_offset, SEEK_SET);
    //if ((wamt = fwrite(&(gff->types), 1, sizeof(gff_type_header_t), gff->file))
            //!= sizeof(gff_type_header_t)) {
    if ((fwrite(&(gff->num_types), 1, sizeof(uint16_t), gff->file))
            != sizeof(uint16_t)) {
        fatal("Unable to write types header!\n");
    }

    fseek(gff->file, gff->header.toc_location + gff->toc.types_offset + 2L, SEEK_SET);
    for (int i = 0; i < gff->num_types; i++) {
        gff_chunk_entry_t *chunk = gff->chunks[i];
        fwrite(&(chunk->chunk_type), 1, sizeof(uint32_t), gff->file);
        fwrite(&(chunk->chunk_count), 1, sizeof(uint32_t), gff->file);
        if (chunk->chunk_count & GFFSEGFLAGMASK) {
            error("SEGMENT TOC Writting not implemented.\n");
            exit(1);
            fseek(gff->file, sizeof(seg_header_t), SEEK_SET);

            //fread(&(chunk->segs.segs), 1, seg_header.num_entries * sizeof(gff_seg_entry_t), gff->file);
            fseek(gff->file, chunk->segs.header.num_entries * sizeof(gff_seg_entry_t), SEEK_CUR);
        } else {
            //printf("%d: write %d chunks\n", i, chunk->chunk_count);
            fwrite(&(chunk->chunks), 1, sizeof(gff_chunk_header_t) * chunk->chunk_count, gff->file);
        }
    }

    //printf("toc_length = %d, free_list-offset = %d\n", gff->header.toc_length, gff->toc.free_list_offset);
    size_t bytes_to_write = gff->header.toc_length - gff->toc.free_list_offset;
    if (bytes_to_write > TOC_BUF_SIZE) {
        error ("overflowed buffer, make a loop!");
        return 0;
    }
    fwrite(buf, 1, bytes_to_write, gff->file);

    return 1;
}

size_t gff_add_chunk(const int idx, const int type_id, int res_id, char *buf, const size_t len) {
    gff_chunk_entry_t *entry = NULL;
    gff_file_t *gff = open_files + idx;
    int chunk_idx;

    for (chunk_idx = 0; !entry && chunk_idx < gff->num_types; chunk_idx++) {
        if ((gff->chunks[chunk_idx]->chunk_type & GFFMAXCHUNKMASK) == type_id) {
            entry = gff->chunks[chunk_idx];
            break;
        }
    }

    if (!entry) {
        error("type not found.\n");
        return 0;
    } // Didn't find...

    if (entry->chunk_count & GFFSEGFLAGMASK) {
        error("can't add to a segment.\n");
        return 0;
    } 

    // It looks like it is best to just increase the amount (reuse is usually determined internally to the resource.)
    // Find out if TOC is at the end of the file.
    fseek(gff->file, 0L, SEEK_END);
    size_t size = ftell(gff->file);
    size_t write_pos = (size == (gff->header.toc_length + gff->header.toc_location)) ? gff->header.toc_location : size;
    for (size_t i = 0; i < entry->chunk_count; i++) {
        if (entry->chunks[i].id == res_id) {
            error("id %d for that type already exists!\n", res_id);
            return 0;
        }
    }
    entry->chunk_count++;
    entry = realloc(entry, 8L + entry->chunk_count * sizeof(gff_chunk_header_t));
    entry->chunks[entry->chunk_count - 1].id = res_id;
    entry->chunks[entry->chunk_count - 1].location = gff->header.toc_location;
    entry->chunks[entry->chunk_count - 1].length = len;

    write_pos += len;

    gff->header.toc_location = write_pos;
    gff->header.toc_length += sizeof(gff_chunk_header_t);
    gff->toc.free_list_offset += sizeof(gff_chunk_header_t);
    gff->chunks[chunk_idx] = entry; // in case a new memory location was allocated.

    // Now write the new chunk to the end of the file.
    fseek(gff->file, entry->chunks[entry->chunk_count - 1].location, SEEK_SET);
    fwrite(buf, 1, len, gff->file);

    // Write out the TOC to the end of the file (you will need to re-alloc the TOC header.)
    return write_toc(idx);
}

size_t gff_read_chunk_length(int idx, int type_id, int res_id, void *read_buf, const size_t len) {
    gff_chunk_header_t chunk = gff_find_chunk_header(idx, type_id, res_id);
    return chunk.length;
}

extern size_t gff_read_chunk(int idx, gff_chunk_header_t *chunk, void *read_buf, const size_t len) {
    if (chunk->length > len) {
        return -1;
    }

    fseek(open_files[idx].file, chunk->location, SEEK_SET);
    return fread(read_buf, 1, chunk->length, open_files[idx].file);
}

extern size_t gff_read_chunk_piece(int idx, gff_chunk_header_t *chunk, void *read_buf, const size_t len) {
    size_t min = chunk->length > len ? len : chunk->length;
    fseek(open_files[idx].file, chunk->location, SEEK_SET);
    return fread(read_buf, 1, min, open_files[idx].file);
}

size_t gff_read_raw_bytes(int idx, int type_id, int res_id, void *read_buf, const size_t len) {
    gff_chunk_header_t chunk = gff_find_chunk_header(idx, type_id, res_id);
    return gff_read_chunk(idx, &chunk, read_buf, len);
}

size_t gff_write_chunk(const int idx, const gff_chunk_header_t chunk, const char *path) {
    fseek(open_files[idx].file, chunk.location, SEEK_SET);
    return fwrite(path, 1, chunk.length, open_files[idx].file);
}

int gff_write_raw_bytes(int idx, int type_id, int res_id, const char *path) {
    int amt = 0;
    char *data;
    FILE *file;

    gff_chunk_header_t chunk = gff_find_chunk_header(idx, type_id, res_id);
    data = malloc(chunk.length);

    if (!gff_read_chunk(idx, &chunk, data, chunk.length) ) { return 0; }

    file = fopen(path, "w+");
    if (!file) { return 0; }

    amt = fwrite(data, 1, chunk.length, file);

    fclose (file);
    return amt;
}

/*
 * Return the number of ids within a certain file (idx) and type (type_id).
 */
unsigned int gff_get_resource_length(int idx, int type_id) {
    unsigned int sum = 0;
    gff_seg_header_t  *seg_header;

    for (int i = 0; i < open_files[idx].num_types; i++) {
        if ((open_files[idx].chunks[i]->chunk_type & GFFMAXCHUNKMASK) != type_id) { continue; }
        if ((open_files[idx].chunks[i]->chunk_count) & GFFSEGFLAGMASK) {
            seg_header = (gff_seg_header_t*)&(open_files[idx].chunks[i]->chunks[0]);
            for (int j = 0; j < seg_header->segRef.num_entries; j++) {
                sum += seg_header->segRef.entries[j].consec_chunks;
            }
        } else {
            sum = open_files[idx].chunks[i]->chunk_count;
        }
    }

    return sum;
}

/*
 * Copies a list of ids into buf.
 * The caller MUST allocate the correct amount before calling.
 * use gff_get_resource_length to know the amt!
 * returns -1 if there is an error, otherwise returns the number of ids copied in.
 */
size_t gff_get_resource_ids(int idx, int type_id, unsigned int *ids) {
    int pos = 0;
    unsigned char *cptr;
    gff_seg_header_t  *seg_header;
    gff_chunk_header_t *chunk_header;
    
    for (int i = 0; i < open_files[idx].num_types; i++) {
        if ((open_files[idx].chunks[i]->chunk_type & GFFMAXCHUNKMASK) != type_id) { continue; }
        if (open_files[idx].chunks[i]->chunk_count & GFFSEGFLAGMASK) {
            seg_header = (gff_seg_header_t*)&(open_files[idx].chunks[i]->chunks[0]);
            for (int j = 0; j < seg_header->segRef.num_entries; j++) {
                for (int id_offset = 0; id_offset < seg_header->segRef.entries[j].consec_chunks; id_offset++) {
                    ids[pos++] = seg_header->segRef.entries[j].first_id + id_offset;
                }
            }
        } else {
            //TODO: migrate the cpt to the proper structs.
            cptr = (void*)open_files[idx].chunks[i];
            for (int j = 0; j < open_files[idx].chunks[i]->chunk_count; j++) {
                chunk_header = (void*)(cptr + sizeof(gff_chunk_list_header_t) + (j * sizeof(gff_chunk_header_t)));
                ids[pos++] = chunk_header->id;
            }
        }
    }

    return pos;
}

/*
 * Returns a list of ALL ids associated with file(idx) and type_id.
 * To know the length, please use gff_get_resource_length.
 */
extern unsigned int* gff_get_id_list(int idx, int type_id) {
    unsigned int *ids = NULL;
    unsigned int len = gff_get_resource_length(idx, type_id);

    ids = malloc(sizeof(unsigned int) * len);

    if (ids == NULL) { return NULL; }

    gff_get_resource_ids(idx, type_id, ids);

    return ids;
}

void gff_get_gff_type_name(unsigned int gff_type, char *type) {
    switch(gff_type) {
        case GFF_FORM: strcpy(type, "FORM: Internal Format"); break;
        case GFF_GFFI: strcpy(type, "GFFI: Internal Format"); break; 
        case GFF_GFRE: strcpy(type, "GFRE: Internal Format"); break; 
        case GFF_GFFOC: strcpy(type, "GFFOC: Internal Format"); break; 
        case GFF_ACCL: strcpy(type, "ACCL: Accelerator resource"); break; 
        case GFF_ADV : strcpy(type, "ADV: AIL Audio Driver"); break; 
        case GFF_APFM: strcpy(type, "APFM: Application Frame Resource "); break; 
        case GFF_BMAP: strcpy(type, "BMAP: 3Dx Bump map (GIL bimap table, 1 bitmap) "); break; 
        case GFF_BMP : strcpy(type, "BMP: Bitmap (GIL bitmap table, 1+ bitmaps)"); break; 
        case GFF_CBMP: strcpy(type, "CBMP: Color Bitmap "); break; 
        case GFF_BVOC: strcpy(type, "BVOC: Background play samples"); break; 
        case GFF_BUTN: strcpy(type, "BUtn: Button resource"); break; 
        case GFF_CMAP: strcpy(type, "CMAP: Color map table"); break; 
        case GFF_DADV: strcpy(type, "DADV: AIL and .COM drivers"); break; 
        case GFF_DATA: strcpy(type, "DATA: Looks like a table for what class combos are allowed"); break; 
        case GFF_DBOX: strcpy(type, "DBOX: Dialog box resource"); break; 
        case GFF_DRV : strcpy(type, "DRV: Run-time loadable code/data module"); break; 
        case GFF_EDAT: strcpy(type, "EDAT: 3DX Environment database"); break; 
        case GFF_EBOX: strcpy(type, "EBOX: Edit Box resource"); break; 
        case GFF_ETAB: strcpy(type, "ETAB: Object entry table"); break; 
        case GFF_FONT: strcpy(type, "FONT: Font (GIL font)"); break; 
        case GFF_FVOC: strcpy(type, "FVOC: Foreground play samples"); break; 
        case GFF_GMAP: strcpy(type, "GMAP: Region map Flags"); break; 
        case GFF_GPL : strcpy(type, "GPL: Compile GPL files"); break; 
        case GFF_GPLX: strcpy(type, "GPLX: GPL index file (GPLSHELL)"); break; 
        case GFF_ICON: strcpy(type, "ICON: Icon resource (GIL bitmap table, 1-4 bitmaps)"); break; 
        case GFF_MAS : strcpy(type, "MAX: Compiled MAS (GPL master) files)"); break; 
        case GFF_MENU: strcpy(type, "MENU: Menu resource"); break; 
        case GFF_MONR: strcpy(type, "MONR: MONR data: monsters by region ids and level (used in request.c DARKSUN)"); break; 
        case GFF_MGFFL: strcpy(type, "MGFFL: Global timbre library"); break; 
        case GFF_MSEQ: strcpy(type, "MSEQ: XMIDI sequence Files (.XMI)"); break; 
        case GFF_OMAP: strcpy(type, "OMAP: Opacty map (GIL bitmap table, 1 bitmap)"); break; 
        case GFF_PAL : strcpy(type, "PAL: VGA 256 color palette"); break; 
        case GFF_POBJ: strcpy(type, "POBJ: PolyMesh object database"); break; 
        case GFF_RMAP: strcpy(type, "RMAP: Region tile map"); break; 
        case GFF_SCMD: strcpy(type, "SCMD: Animation script command table"); break; 
        case GFF_SBAR: strcpy(type, "SBAR: Scoll-bar resource"); break; 
        case GFF_SINF: strcpy(type, "SINF: Sound card info"); break; 
        case GFF_SJMP: strcpy(type, "SJMP: OENGINE animation script jump table"); break; 
        case GFF_TEXT: strcpy(type, "TEXT: Text resource"); break; 
        case GFF_TILE: strcpy(type, "TILE: Tile graphic (GIL bitmap table, 1 bitmap)"); break; 
        case GFF_TMAP: strcpy(type, "TMAP: Texture map (GIL bitmap table, 1 bitmap)"); break; 
        case GFF_TXRF: strcpy(type, "TXRF: Texture map (GIL bitmap table, 1 bitmap)"); break; 
        case GFF_WIND: strcpy(type, "WIND: Windows resource"); break; 
        case GFF_ACF: strcpy(type, "ACF: Cinematic Binary Script File"); break; 
        case GFF_BMA: strcpy(type, "BMA: Cinematic Binary File"); break; 
        case GFF_WALL: strcpy(type, "WALL: Wall data"); break;
        case GFF_OJFF: strcpy(type, "OJFF: Object?  data?  (unsure.)"); break;
        case GFF_RDFF: strcpy(type, "RDFF data"); break;
        case GFF_FNFO: strcpy(type, "FNFO: appears to have the object data for the game (items, creatures, etc...)");
                break;
        case GFF_IT1R: strcpy(type, "IT1R data"); break;
        case GFF_NAME: strcpy(type, "NAME data"); break;
        case GFF_ALL: strcpy(type, "ALL data(ALL the game data?)"); break;
        case GFF_VECT: strcpy(type, "VECT Data"); break;
        case GFF_MERR: strcpy(type, "MERR Data: text error file"); break;
        case GFF_PSEQ: strcpy(type, "PSEQ Data: PCSPKR"); break;
        case GFF_FSEQ: strcpy(type, "FSEQ Data: FM"); break;
        case GFF_LSEQ: strcpy(type, "LSEQ Data: LAPC"); break;
        case GFF_GSEQ: strcpy(type, "GSEQ Data: General MIDI"); break;
        case GFF_CSEQ: strcpy(type, "CSEQ Data: Clock sequences"); break;
        case GFF_CMAT: strcpy(type, "CMAT Data"); break;
        case GFF_CPAL: strcpy(type, "CPAL Data"); break;
        case GFF_PLYL: strcpy(type, "PLYL Data"); break;
        case GFF_VPLY: strcpy(type, "VPLY Data"); break;
        case GFF_SPIN: strcpy(type, "SPIN Data: The text field of a spell."); break;
        case GFF_PORT: strcpy(type, "PORT Data: The portraits for dialogs."); break;
        case GFF_SPST: strcpy(type, "SPST Data: The spell list."); break;
        case GFF_PSST: strcpy(type, "PSST Data: The psionic list."); break;
        case GFF_CHAR: strcpy(type, "CHAR Data: The character data."); break;
        case GFF_PSIN: strcpy(type, "PSIN Data: The psionic and sphere lists."); break;
        case GFF_CACT: strcpy(type, "CACT Data: The id for a player."); break;
        case GFF_STXT: strcpy(type, "STXT Data: The name of the save file."); break;
        case GFF_ETME: strcpy(type, "ETME Data: Copyright notice, if you see this it means don't use it."); break;
        case GFF_SAVE: strcpy(type, "SAVE Data: "); break;
        default: 
            sprintf(type, "UNKNOWN TYPE: '%c' '%c' '%c' '%c'",
                (gff_type >> 0) & 0x00FF,
                (gff_type >> 8) & 0x00FF,
                (gff_type >> 16) & 0x00FF,
                (gff_type >> 24) & 0x00FF);
            break;
    }
}

size_t gff_get_palette_id(int idx, int palette_num) {
    if (palette_num < 0 || palette_num >= open_files[idx].num_palettes) {
        return -1;
    }

    return open_files[idx].start_palette_index + palette_num;
}

void gff_cleanup() {
    for (int i = 0; i < NUM_FILES; i++) {
        gff_close(i);
    }
}

static void gff_close_file(gff_file_t *gff) {
    if (gff->filename) {
        free(gff->filename);
        gff->filename = NULL;
    }
    if (gff->map) {
        free(gff->map);
        gff->map = NULL;
    }
    if (gff->chunks) {
        for (int i = 0; i < gff->num_types; i++) {
            if (gff->chunks[i]) {
                free(gff->chunks[i]);
                gff->chunks[i] = NULL;
            }
        }
        free(gff->chunks);
        gff->chunks = NULL;
    }
    if (gff->file) {
        fclose(gff->file);
    }
    if (gff->pals) {
        free(gff->pals);
        gff->pals = NULL;
    }
    if (gff->entry_table) {
        free(gff->entry_table);
        gff->entry_table = NULL;
    }
    if (gff->pals) {
        free(gff->pals);
        gff->pals = NULL;
    }
    gff->file = NULL;
    gff->filename = NULL;
}

void gff_close (int gff_file) {
    if (gff_file < 0 || gff_file > NUM_FILES) { return; }

    gff_close_file(open_files + gff_file );

}
