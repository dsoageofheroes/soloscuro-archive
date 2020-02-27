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

typedef struct {
  // --- File Header	    ---
  uint32_t   identity;	    //	0
  uint32_t   version;	    //	4
  uint32_t   dataLocation;     //	8
  uint32_t   tocLocation;	    // 12
  uint32_t   tocLength;	    // 16
  uint32_t   fileFlags;	    // 20
  uint32_t   reserve1;	    // 24
  // --- End of File Header ---
  // --- Empty TOC follows  ---
  uint32_t   typesLocation;    // 28    0
  uint32_t   freeListLocation; // 32    4
  uint16_t   numTypes;	    // 36    8
  uint16_t   freeChunks;	    // 38   10
  // --- End of Empty TOC   ---
} gff_empty_file_t;

static gff_chunk_list_t* get_gffi_header(gff_file_t *file);
static gff_type_header_t* get_type_header(int idx);
static int get_next_idx(char *name);

int gff_get_master() {
    return master_gff;
}

static char* strtolwr(char *str) {

    for (char *c = str; *c; c++) {
        *(c) = tolower(*c);
    }

    return str;
}

void gff_print_something()
{
    printf("Why does this not print?");
}

// Precondition, name is in lowercase!
int gff_find_index(char *name) {
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
int get_next_idx(char *name) {
    int i;

    if (gff_find_index(name) > -1) {
        fprintf(stderr, "'%s' is already loaded!\n", name);
        return -1; // We already have that file!
    }

    if (strcmp(name, "resource.gff") == 0) { return RESOURCE_GFF_INDEX; }
    if (strcmp(name, "segobjex.gff") == 0) { return OBJEX_GFF_INDEX; }
    if (strcmp(name, "gpldata.gff") == 0) { return DSLDATA_GFF_INDEX; }
    if (strcmp(name, "charsave.gff") == 0) { return CHARSAVE_GFF_INDEX; }
    if (strcmp(name, "darksave.gff") == 0) { return DARKSAVE_GFF_INDEX; }
    if (strcmp(name, "cine.gff") == 0) { return CINE_GFF_INDEX; }
    if (strcmp(name, "darkrun.gff") == 0) { return DARKRUN_GFF_INDEX; }

    for (i = REST_GFF_INDEX; i < NUM_FILES; i++) {
        if (!open_files[i].data) {
            return i;
        }
    }

    fprintf(stderr, "Number of GFF slots are full!\n");
    return -1;
}

void gff_init() {
    int i = 0;

    for (i = 0; i < NUM_FILES; i++) {
        memset(open_files+i, 0, sizeof(gff_file_t));
        open_files[i].len = 0;
    }

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

    printf("Loading GFFs from: %s\n", path);
    if ((dir = opendir (path)) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if (is_gff_filename(ent->d_name)) {
                snprintf(buf, BUF_SIZE, "%s/%s", path, ent->d_name);
                //printf("Loading GFF: '%s'\n", buf);
                gff_open(buf);
            }
        }
        closedir (dir);
    } else {
        fprintf(stderr, "Unable to open directory: '%s'\n", path);
    }
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

int gff_create(const char *pathName) {
    uint32_t 	  len;
    int idx;
    gff_empty_file_t   emptyFile;
    char *filename = strdup(pathName);

	// Write the empty file struct with known offset
	// values. This is the whole file structure of
	// an "empty" file.
	emptyFile.identity	    = GT_GFFI;
	emptyFile.version	    = GFFVERSION;
	emptyFile.dataLocation	= 28L;
	emptyFile.tocLocation	= 28L;
	emptyFile.tocLength	    = 12L;
	emptyFile.fileFlags	    =  0L;
	emptyFile.reserve1	    =  0L;

	// Following 3 are offsets from beginning of TOC
	emptyFile.typesLocation     =  8L;
	emptyFile.freeListLocation  = 10L;

	// We have none of anything.
	emptyFile.numTypes	    =  0;
	emptyFile.freeChunks	    =  0;

	len = (uint32_t)sizeof(gff_empty_file_t);

    idx = get_next_idx(strtolwr(filename));
    // Many things can cause, see stderr.
    if (idx == -1) { 
        free(filename);
        return idx; 
    }

    open_files[idx].data = malloc(len);
    open_files[idx].filename = filename;
    memcpy(open_files[idx].data, &emptyFile, len);

    return idx;
}

int gff_update(const char *path, int id) {
    int len = 0;

    FILE *file = fopen (path, "w+");
    len = fwrite(open_files[id].data, open_files[id].len, 1, file);
    fclose(file);

    return len;
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

int gff_open(const char *pathName) {
    int idx, len;
    ssize_t read_amt;
    FILE *file;
    //int buf[BUF_SIZE];
    char *filename = strtolwr(get_filename_from_path(pathName));

    // First check we can get an index for the file.
    idx = get_next_idx(filename);
    if (idx == -1) { 
        free(filename);
        return idx; 
    }

    file = fopen(pathName, "rb+");
    if (!file) { return -1; }
    len = 0;
    fseek(file, 0L, SEEK_END);
    len = ftell(file);
    fseek(file, 0L, SEEK_SET);

    //printf("Detected file size of '%s': %d\n", filename, len);
    open_files[idx].len = len;
    open_files[idx].data = malloc(open_files[idx].len);
    open_files[idx].palettes = NULL;
    open_files[idx].num_palettes = 0;
    open_files[idx].num_objects = -1;
    open_files[idx].entry_table = NULL;

    read_amt = fread(open_files[idx].data, 1, open_files[idx].len, file);

    if (read_amt < open_files[idx].len) {
        fprintf(stderr, "ERROR: unable to read the entire file '%s'\n", filename);
        free (open_files[idx].data);
        open_files[idx].len = 0;
        open_files[idx].data = NULL;
        return -1;
    }

    fclose(file);
    if (is_master_name(filename)) { master_gff = idx; }

    open_files[idx].filename = filename;
    open_files[idx].start_palette_index = gff_get_number_of_palettes();
    open_files[idx].palettes = create_palettes(idx, &(open_files[idx].num_palettes));
    debug("'%s' loaded as '%s' with id: %d\n", pathName, open_files[idx].filename, idx);

    return idx;
}

static gff_type_header_t* get_type_header(int idx) {
    if (open_files[idx].data == NULL) { return NULL; }

    gff_file_header_t *header = open_files[idx].data;
    unsigned char *cptr = open_files[idx].data;
    gff_toc_header_t *toc_header = (void*)(cptr + header->tocLocation);
    gff_type_header_t *type_header = NULL;

    cptr = (void*)toc_header;
    type_header = (void*)(cptr + toc_header->typesOffset);

    return type_header;
}

int gff_get_number_of_types(int idx) {
    if (open_files[idx].data == NULL) { return -1; }

    return get_type_header(idx)->numTypes;
}

int gff_get_type_id(int idx, int type_index) {
    int i;
    gff_type_header_t *type_header = get_type_header(idx);
    unsigned char *cptr = (void*) type_header;
    gff_chunk_list_t *chunk_list = (void*)(cptr + 2L);

    if (open_files[idx].data == NULL || type_index < 0 
            || type_index >= type_header->numTypes) { 
        return -1;  // failure.
    }

    // Go to the index.
    cptr = (void*)chunk_list;
    for (i = 0; i < type_index; i++) {
        if (chunk_list->chunkCount & GFFSEGFLAGMASK) {
            cptr += GFFCHUNKLISTHEADERSIZE;
            cptr += ((gff_seg_header_t*)(cptr))->segRef.numEntries * GFFSEGREFENTRYSIZE;
            cptr += 12L;
        } else {
            cptr += GFFCHUNKLISTHEADERSIZE;
            cptr += ((uint32_t)(chunk_list->chunkCount & GFFMAXCHUNKMASK) * GFFCHUNKHEADERSIZE);
        }
        chunk_list = (void*) cptr;
    }
    
    return chunk_list->chunkType & GFFMAXCHUNKMASK;
}

/*
 * Finds and returns a pointer to the part that represents the data.
 * len is set to the number of bytes.
 * WARNING WARNING WARNING DANGER DANGER DANGER:
 * Data segments are next to each other, if you go out of bounds, all sorts
 * of strange things will happen.  Be careful of the bounds of the returned pointer!
 */
char* gff_get_raw_bytes(int idx, int type_id, int res_id, unsigned long *len) {
    gff_seg_header_t  *seg_header;
    gff_chunk_header_t *chunk_header;
    gff_chunk_list_t* chunk_list;
    unsigned char *cptr, *seg_ptr;

    chunk_list = search_for_chunk_by_name(open_files + idx, type_id);
    if (chunk_list == NULL) { return NULL; }

    cptr = (void*)chunk_list;
    if (chunk_list->chunkCount & GFFSEGFLAGMASK) {
        seg_header = (gff_seg_header_t*)&(chunk_list->chunks[0]);
        seg_ptr = (unsigned char*)get_gffi_header(open_files + idx);// Right now seg_ptr is a tmp.
        gff_chunk_header_t *gffi_chunk_header = (gff_chunk_header_t*)(seg_ptr + GFFCHUNKLISTHEADERSIZE +
            (seg_header->segLocId*GFFCHUNKHEADERSIZE));
        seg_ptr = (unsigned char*)open_files[idx].data; 
        seg_ptr += gffi_chunk_header->chunkDataLocation;// Maybe where the seg pointer is!
        cptr += GFFCHUNKLISTHEADERSIZE;
        int ndx = 0;
        for (int j = 0; j < seg_header->segRef.numEntries; j++) {
            int tmpId = (seg_header->segRef.entries[j].firstId);
            int offset = 4L + (GFFSEGLOCENTRYSIZE * ndx);
            if (res_id >= tmpId && res_id <= (tmpId + seg_header->segRef.entries[j].consecChunks)) {
                gff_seg_loc_entry_t * seg_loc = (gff_seg_loc_entry_t*)(seg_ptr + offset +
                    (res_id-tmpId)*GFFSEGLOCENTRYSIZE);
                *len = seg_loc->segLength;
                cptr = open_files[idx].data;
                return (void*) (cptr + seg_loc->segOffset);
            }
            ndx += seg_header->segRef.entries[j].consecChunks;
        }
    } else {
        cptr = (void*)chunk_list;
        for (int j = 0; j < chunk_list->chunkCount; j++) {
            chunk_header = (void*)(cptr + GFFCHUNKLISTHEADERSIZE + (j * GFFCHUNKHEADERSIZE));
            if (chunk_header->chunkId == res_id) {
                *len = chunk_header->chunkLength;
                cptr = open_files[idx].data;
                return (void*) (cptr + chunk_header->chunkDataLocation);
            }
        }
    }

    // didn't find...
    return NULL;
}

int gff_write_raw_bytes(int idx, int type_id, int res_id, const char *path) {
    unsigned long len = 0;
    int amt = 0;
    char *data;
    FILE *file;

    data = gff_get_raw_bytes(idx, type_id, res_id, &len);
    if (len < 1 || data == NULL) { return 0; }

    file = fopen(path, "w+");
    if (!file) { return 0; }

    amt = fwrite(data, 1, len, file);

    fclose (file);
    return amt;
}

/*
 * Return the number of ids within a certain file (idx) and type (type_id).
 */
unsigned int gff_get_gff_type_length(int idx, int type_id) {
    unsigned int sum = 0;
    gff_seg_header_t  *seg_header;
    gff_chunk_list_t* chunk_list = search_for_chunk_by_name(open_files + idx, type_id);

    if (chunk_list == NULL) { return 0; }

    if (chunk_list->chunkCount & GFFSEGFLAGMASK) {
        seg_header = (gff_seg_header_t*)&chunk_list->chunks[0];
        for (int j = 0; j < seg_header->segRef.numEntries; j++) {
            sum += seg_header->segRef.entries[j].consecChunks;
        }
    } else {
        sum = chunk_list->chunkCount;
    }

    return sum;
}

/*
 * Returns a list of ALL ids associated with file(idx) and type_id.
 * To know the length, please use gff_get_gff_type_length.
 */
extern unsigned int* gff_get_id_list(int idx, int type_id) {
    unsigned int *ids = NULL;
    int pos = 0, len = gff_get_gff_type_length(idx, type_id);
    unsigned char *cptr;
    gff_chunk_list_t* chunk_list;
    gff_seg_header_t  *seg_header;
    gff_chunk_header_t *chunk_header;

    if (len == 0) { return ids; }

    chunk_list = search_for_chunk_by_name(open_files + idx, type_id);
    if (chunk_list == NULL) { return NULL; }
    ids = malloc(sizeof(unsigned int) * len);
    if (ids == NULL) { return NULL; }

    if (chunk_list->chunkCount & GFFSEGFLAGMASK) {
        seg_header = (gff_seg_header_t*)&chunk_list->chunks[0];
        for (int j = 0; j < seg_header->segRef.numEntries; j++) {
            for (int id_offset = 0; id_offset < seg_header->segRef.entries[j].consecChunks; id_offset++) {
                ids[pos++] = seg_header->segRef.entries[j].firstId + id_offset;
            }
        }
    } else {
        cptr = (void*)chunk_list;
        for (int j = 0; j < chunk_list->chunkCount; j++) {
            chunk_header = (void*)(cptr + GFFCHUNKLISTHEADERSIZE + (j * GFFCHUNKHEADERSIZE));
            ids[pos++] = chunk_header->chunkId;
        }
    }

    // If we fail to find any, we fail to find all.
    if (pos < len) {
        free(ids);
        return NULL;
    }

    return ids;
}

void get_gff_type_name(unsigned int gff_type, char *type) {
    switch(gff_type) {
        case GT_FORM: strcpy(type, "FORM: Internal Format"); break;
        case GT_GFFI: strcpy(type, "GFFI: Internal Format"); break; 
        case GT_GFRE: strcpy(type, "GFRE: Internal Format"); break; 
        case GT_GTOC: strcpy(type, "GTOC: Internal Format"); break; 
        case GT_ACCL: strcpy(type, "ACCL: Accelerator resource"); break; 
        case GT_ADV : strcpy(type, "ADV: AIL Audio Driver"); break; 
        case GT_APFM: strcpy(type, "APFM: Application Frame Resource "); break; 
        case GT_BMAP: strcpy(type, "BMAP: 3Dx Bump map (GIL bimap table, 1 bitmap) "); break; 
        case GT_BMP : strcpy(type, "BMP: Bitmap (GIL bitmap table, 1+ bitmaps)"); break; 
        case GT_CBMP: strcpy(type, "CBMP: Color Bitmap "); break; 
        case GT_BVOC: strcpy(type, "BVOC: Background play samples"); break; 
        case GT_BUTN: strcpy(type, "BUtn: Button resource"); break; 
        case GT_CMAP: strcpy(type, "CMAP: Color map table"); break; 
        case GT_DADV: strcpy(type, "DADV: AIL and .COM drivers"); break; 
        case GT_DATA: strcpy(type, "DATA: Looks like a table for what class combos are allowed"); break; 
        case GT_DBOX: strcpy(type, "DBOX: Dialog box resource"); break; 
        case GT_DRV : strcpy(type, "DRV: Run-time loadable code/data module"); break; 
        case GT_EDAT: strcpy(type, "EDAT: 3DX Environment database"); break; 
        case GT_EBOX: strcpy(type, "EBOX: Edit Box resource"); break; 
        case GT_ETAB: strcpy(type, "ETAB: Object entry table"); break; 
        case GT_FONT: strcpy(type, "FONT: Font (GIL font)"); break; 
        case GT_FVOC: strcpy(type, "FVOC: Foreground play samples"); break; 
        case GT_GMAP: strcpy(type, "GMAP: Region map Flags"); break; 
        case GT_GPL : strcpy(type, "GPL: Compile GPL files"); break; 
        case GT_GPLX: strcpy(type, "GPLX: GPL index file (GPLSHELL)"); break; 
        case GT_ICON: strcpy(type, "ICON: Icon resource (GIL bitmap table, 1-4 bitmaps)"); break; 
        case GT_MAS : strcpy(type, "MAX: Compiled MAS (GPL master) files)"); break; 
        case GT_MENU: strcpy(type, "MENU: Menu resource"); break; 
        case GT_MONR: strcpy(type, "MONR: MONR data: monsters by region ids and level (used in request.c DARKSUN)"); break; 
        case GT_MGTL: strcpy(type, "MGTL: Global timbre library"); break; 
        case GT_MSEQ: strcpy(type, "MSEQ: XMIDI sequence Files (.XMI)"); break; 
        case GT_OMAP: strcpy(type, "OMAP: Opacty map (GIL bitmap table, 1 bitmap)"); break; 
        case GT_PAL : strcpy(type, "PAL: VGA 256 color palette"); break; 
        case GT_POBJ: strcpy(type, "POBJ: PolyMesh object database"); break; 
        case GT_RMAP: strcpy(type, "RMAP: Region tile map"); break; 
        case GT_SCMD: strcpy(type, "SCMD: Animation script command table"); break; 
        case GT_SBAR: strcpy(type, "SBAR: Scoll-bar resource"); break; 
        case GT_SINF: strcpy(type, "SINF: Sound card info"); break; 
        case GT_SJMP: strcpy(type, "SJMP: OENGINE animation script jump table"); break; 
        case GT_TEXT: strcpy(type, "TEXT: Text resource"); break; 
        case GT_TILE: strcpy(type, "TILE: Tile graphic (GIL bitmap table, 1 bitmap)"); break; 
        case GT_TMAP: strcpy(type, "TMAP: Texture map (GIL bitmap table, 1 bitmap)"); break; 
        case GT_TXRF: strcpy(type, "TXRF: Texture map (GIL bitmap table, 1 bitmap)"); break; 
        case GT_WIND: strcpy(type, "WIND: Windows resource"); break; 
        case GT_ACF: strcpy(type, "ACF: Cinematic Binary Script File"); break; 
        case GT_BMA: strcpy(type, "BMA: Cinematic Binary File"); break; 
        case GT_WALL: strcpy(type, "WALL: Wall data"); break;
        case GT_OJFF: strcpy(type, "OJFF: Object?  data?  (unsure.)"); break;
        case GT_RDFF: strcpy(type, "RDFF data"); break;
        case GT_FNFO: strcpy(type, "FNFO: appears to have the object data for the game (items, creatures, etc...)");
                break;
        case GT_IT1R: strcpy(type, "IT1R data"); break;
        case GT_NAME: strcpy(type, "NAME data"); break;
        case GT_ALL: strcpy(type, "ALL data(ALL the game data?)"); break;
        case GT_VECT: strcpy(type, "VECT Data"); break;
        case GT_MERR: strcpy(type, "MERR Data: text error file"); break;
        case GT_PSEQ: strcpy(type, "PSEQ Data: PCSPKR"); break;
        case GT_FSEQ: strcpy(type, "FSEQ Data: FM"); break;
        case GT_LSEQ: strcpy(type, "LSEQ Data: LAPC"); break;
        case GT_GSEQ: strcpy(type, "GSEQ Data: General MIDI"); break;
        case GT_CSEQ: strcpy(type, "CSEQ Data: Clock sequences"); break;
        case GT_CMAT: strcpy(type, "CMAT Data"); break;
        case GT_CPAL: strcpy(type, "CPAL Data"); break;
        case GT_PLYL: strcpy(type, "PLYL Data"); break;
        case GT_VPLY: strcpy(type, "VPLY Data"); break;
        case GT_SPIN: strcpy(type, "SPIN Data: The text field of a spell."); break;
        case GT_PORT: strcpy(type, "PORT Data: The portraits for dialogs."); break;
        default: 
            sprintf(type, "UNKNOWN TYPE: '%c' '%c' '%c' '%c'",
                (gff_type >> 0) & 0x00FF,
                (gff_type >> 8) & 0x00FF,
                (gff_type >> 16) & 0x00FF,
                (gff_type >> 24) & 0x00FF);
            break;
    }
}

gff_chunk_list_t* search_for_chunk_by_name(gff_file_t *file, unsigned long name) {
    gff_file_header_t *header = file->data;
    gff_toc_header_t *toc_header = NULL;
    gff_type_header_t *type_header = NULL;
    gff_chunk_list_t *chunk_list = NULL;
    unsigned char *cptr = file->data;
    int i = 0;

    toc_header = (void*)(cptr + header->tocLocation);
    cptr = (void*)toc_header;

    type_header = (void*)(cptr + toc_header->typesOffset);
    cptr = (void*)type_header;

    chunk_list = (void*)(cptr + 2L);
    cptr = (void*)chunk_list;
    for (i = 0; i < type_header->numTypes; i++) {
        if ((chunk_list->chunkType & GFFMAXCHUNKMASK) == name) {
            return chunk_list;
        }
        if (chunk_list->chunkCount & GFFSEGFLAGMASK) {
            cptr += GFFCHUNKLISTHEADERSIZE;
            cptr += ((gff_seg_header_t*)(cptr))->segRef.numEntries * GFFSEGREFENTRYSIZE;
            cptr += 12L;
        } else {
            cptr += GFFCHUNKLISTHEADERSIZE;
            cptr += ((uint32_t)(chunk_list->chunkCount & GFFMAXCHUNKMASK) * GFFCHUNKHEADERSIZE);
        }
        chunk_list = (void*) cptr;
    }

    // Was not found.
    return NULL;
}

gff_chunk_list_t* get_gffi_header(gff_file_t *file) {
    if (!file) { return NULL;}

    if (!file->gffi_data) {
        file->gffi_data = search_for_chunk_by_name(file, GT_GFFI);
    }

    return file->gffi_data;
}

size_t gff_get_palette_id(int idx, int palette_num) {
    if (palette_num < 0 || palette_num >= open_files[idx].num_palettes) {
        return -1;
    }

    return open_files[idx].start_palette_index + palette_num;
}

void gff_print(int idx, FILE *out) {
    gff_file_header_t *header = open_files[idx].data;
    gff_toc_header_t *toc_header = NULL;
    gff_type_header_t *type_header = NULL;
    gff_chunk_list_t *chunk_list = NULL;
    gff_chunk_header_t *chunk_header = NULL;
    gff_seg_header_t  *seg_header = NULL;
    unsigned long gff_type;
    char name_buff[1<<10];
    unsigned char *cptr = open_files[idx].data;
    int i = 0;

    fprintf(out, "Size = %d\n", open_files[idx].len);
    fprintf(out, "identity = 0x%x\n", header->identity);
    fprintf(out, "dataLocation = 0x%x\n", header->dataLocation);
    fprintf(out, "tocLocation = 0x%x\n", header->tocLocation);
    fprintf(out, "tocLength = %d\n", header->tocLength);
    fprintf(out, "fileFlags = 0x%x\n", header->fileFlags);
    fprintf(out, "reserve1 = 0x%x\n", header->reserve1);

    toc_header = (void*)(cptr + header->tocLocation);
    fprintf(out, "toc.typesOffset = %u\n", toc_header->typesOffset);
    fprintf(out, "toc.freeListOffset = %u\n", toc_header->freeListOffset);

    cptr = (void*)toc_header;
    type_header = (void*)(cptr + toc_header->typesOffset);
    fprintf(out, "type_header.numTypes = %d\n", type_header->numTypes);

    cptr = (void*)type_header;
    chunk_list = (void*)(cptr + 2L);
    cptr = (void*)chunk_list;
    for (i = 0; i < type_header->numTypes; i++) {
        gff_type = chunk_list->chunkType & GFFMAXCHUNKMASK;
        get_gff_type_name(gff_type, name_buff);
        fprintf(out, "[%d] : chunkType = %d->%s\n", i, chunk_list->chunkType, name_buff);
        cptr = (void*)chunk_list;

        if (chunk_list->chunkCount & GFFSEGFLAGMASK) {
            seg_header = (gff_seg_header_t*)&chunk_list->chunks[0];
            unsigned char *seg_ptr = (unsigned char*)get_gffi_header(open_files+idx);// Right now seg_ptr is a tmp.
            gff_chunk_header_t *gffi_chunk_header = (gff_chunk_header_t*)(seg_ptr + GFFCHUNKLISTHEADERSIZE +
                (seg_header->segLocId*GFFCHUNKHEADERSIZE));
            seg_ptr = (unsigned char*)open_files[idx].data; 
            seg_ptr += gffi_chunk_header->chunkDataLocation;// Maybe where the seg pointer is!
            cptr += GFFCHUNKLISTHEADERSIZE;
            fprintf(out, "  segCount = %d, segLocId = %d, numEntries = %d\n", seg_header->segCount,
                    seg_header->segLocId, seg_header->segRef.numEntries);
            int ndx = 0;
            // I need to review g30main.c line 652 for more details.
            for (int j = 0; j < seg_header->segRef.numEntries; j++) {
                int tmpId = (seg_header->segRef.entries[j].firstId);
                int offset = 4L + (GFFSEGLOCENTRYSIZE * ndx);
                fprintf(out, "  [%d]: id = %d, SegLocOffset = %d, consecChunks = %d\n", j, tmpId, offset,
                    seg_header->segRef.entries[j].consecChunks);
                for (int k = 0; k < seg_header->segRef.entries[j].consecChunks; k++) {
                    unsigned long len = 0;
                    gff_get_raw_bytes(idx, gff_type, tmpId + k, &len);
                    gff_seg_loc_entry_t * seg_loc = (gff_seg_loc_entry_t*)(seg_ptr + offset + k*GFFSEGLOCENTRYSIZE);
                    fprintf(out, "    offset = %u, length = %lu\n", seg_loc->segOffset, len);
                    /*
                    if (gff_type == GT_SPIN) {
                        fprintf(stderr, "text: '%s'\n", (char*)data);
                    }
                    */
                }
                ndx += seg_header->segRef.entries[j].consecChunks;
            }

            cptr += ((gff_seg_header_t*)(cptr))->segRef.numEntries * GFFSEGREFENTRYSIZE;
            cptr += 12L;
        } else {
            fprintf(out, "  %d regular chunks detected.\n", chunk_list->chunkCount);
            for (int j = 0; j < chunk_list->chunkCount; j++) {
                chunk_header = (void*)(cptr + GFFCHUNKLISTHEADERSIZE + (j * GFFCHUNKHEADERSIZE));
                fprintf(out, "  [%d]: length = %d\n", chunk_header->chunkId, chunk_header->chunkLength);
            }

            cptr += GFFCHUNKLISTHEADERSIZE;
            cptr += ((uint32_t)(chunk_list->chunkCount & GFFMAXCHUNKMASK) * GFFCHUNKHEADERSIZE);
        }
        chunk_list = (void*) cptr;
    }
}

void gff_cleanup() {
    for (int i = 0; i < NUM_FILES; i++) {
        gff_close(i);
    }
}

void gff_close (int gff_file) {
    if (gff_file < 0 || gff_file >= NUM_FILES) { return; }

    if (open_files[gff_file].data) {
        free(open_files[gff_file].data);
    }
    if (open_files[gff_file].filename) {
        free(open_files[gff_file].filename);
    }
    if (open_files[gff_file].map) {
        free(open_files[gff_file].map);
    }
    open_files[gff_file].filename = NULL;
    open_files[gff_file].data = NULL;
    open_files[gff_file].map = NULL;
    open_files[gff_file].palettes = NULL;
    open_files[gff_file].gffi_data = NULL;
    open_files[gff_file].len = 0;
}


gff_monster_entry_t* gff_load_monster(int region_id, int monster_id) {
    if (monster_id < 0 || monster_id > MAX_MONSTERS_PER_REGION) { return NULL; }
    unsigned long len;
    gff_monster_region_t *mr = (gff_monster_region_t*)gff_get_raw_bytes(RESOURCE_GFF_INDEX, GT_MONR, 1, &len);
    if (!mr) {return NULL;}

    for (int i = 0; i < len/sizeof(gff_monster_entry_t); i++) {
        if (mr->region == i) {
            return mr->monsters + monster_id;
        }
    }

    printf("mr->region = %d\n", mr->region);
    for (int j = 0; j < MAX_MONSTERS_PER_REGION; j++) {
        printf("%d: level %d\n", mr->monsters[j].id, mr->monsters[j].level);
    }
    return NULL;
}
