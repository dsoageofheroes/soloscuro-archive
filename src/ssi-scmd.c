#include "dsl.h"
#include "ssi-scmd.h"
#include <stdlib.h>

static scmd_t* get_script(unsigned char* scmd_entry, const int index) {
    if (scmd_entry == NULL) { return NULL; }
    if (index < 0 || index >= SCMD_MAX_SIZE) {
        error("index for get_script is out of bounds!(%d)\n", index);
        return NULL;
    }
    scmd_t *scmds = (scmd_t*)(scmd_entry + (SCMD_MAX_SIZE * 2));
    uint16_t scmd_idx = *((uint16_t*)scmd_entry + index);
    return scmds + scmd_idx;
}

/*
static void print_scmd(scmd_t *scmd) {
    printf("bmp_idx = %d, delay = %d, flags = 0x%x, xoffset = %d, yoffset = %d, "
        "xoffsethot = %d, yoffsethot = %d, soundidx = %d\n",
        scmd->bmp_idx,
        scmd->delay,
        scmd->flags,
        scmd->xoffset,
        scmd->yoffset,
        scmd->xoffsethot,
        scmd->yoffsethot,
        scmd->soundidx
    );
}
*/

static unsigned char scmds[SCMD_MAX][SCMD_MAX];
static scmd_t empty_scmd = {
    .bmp_idx = 0,
    .delay = 0,
    .flags = SCMD_LAST,
    .xoffset = 0,
    .yoffset = 0,
    .xoffsethot = 0,
    .yoffsethot = 0,
    .soundidx = 0
};

extern scmd_t* ssi_scmd_empty() {
    return &empty_scmd;
}

extern scmd_t* ssi_scmd_get(const int gff_idx, const int res_id, const int index) {
    if (res_id <= 0 || res_id >= SCMD_MAX) { return &empty_scmd; } // needs a better check...

    gff_chunk_header_t chunk = gff_find_chunk_header(gff_idx, GFF_SCMD, res_id);
    if (chunk.length > SCMD_MAX) {
        error("chunk length (%d) is larger than SCMD_MAX (%d)\n", chunk.length, SCMD_MAX);
        exit(1);
    }
    gff_read_chunk(gff_idx, &chunk, scmds[res_id], SCMD_MAX);
    return get_script(scmds[res_id], index);
}

extern int ssi_scmd_is_default(const scmd_t *scmd, const int scmd_index) {
    if (scmd == NULL) { return 0; }
    if (scmd_index < 0 || scmd_index >= SCMD_MAX_SIZE) {
        error("index for get_script is out of bounds!(%d)\n", scmd_index);
        return 0;
    }
    uint16_t scmd_idx = *((uint16_t*)(((unsigned char *)scmd) + scmd_index));
    return scmd_idx == 0;
}
