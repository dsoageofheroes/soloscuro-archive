// These are for passive objects that you do not direct interact with
// e.g.: statues, random rocks, bone wall in the arena.
#ifndef PASSIVE_H
#define PASSIVE_H

#include <stdint.h>
#include "ds-scmd.h"
#include "gff-map.h"

typedef struct passive_s {
    uint8_t flags;      // flags
    uint16_t entry_id;  // object entry id for GPL/DSL
    uint16_t bmp_id;    // the BMP/image id/index (can be a collection of frames.)
    int16_t bmpx;       // bitmap's x coordinate
    int16_t bmpy;       // bitmap's y coordinate
    int16_t xoffset;    // bitmap offset x
    int16_t yoffset;    // bitmap offset y
    uint16_t mapx;      // object's x position in the region
    uint16_t mapy;      // object's y position in the region
    int16_t mapz;       // object's z position in the region
    uint16_t scmd_flags;//
    scmd_t *scmd;       // the script
    void *data;         // used for special data the UI needs. (e.g. SDL)
} passive_t;

void passive_load_from_etab(passive_t *passive, gff_map_object_t *entry_table, uint32_t id);

#endif
